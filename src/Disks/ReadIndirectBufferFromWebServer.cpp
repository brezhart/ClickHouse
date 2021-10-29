#include "ReadIndirectBufferFromWebServer.h"

#include <base/logger_useful.h>
#include <base/sleep.h>
#include <Core/Types.h>
#include <IO/ReadWriteBufferFromHTTP.h>
#include <IO/ConnectionTimeoutsContext.h>
#include <IO/WriteBufferFromString.h>
#include <IO/Operators.h>
#include <thread>


namespace DB
{

namespace ErrorCodes
{
    extern const int CANNOT_SEEK_THROUGH_FILE;
    extern const int SEEK_POSITION_OUT_OF_BOUND;
}


ReadIndirectBufferFromWebServer::ReadIndirectBufferFromWebServer(
    const String & url_, ContextPtr context_, size_t buf_size_, const ReadSettings & settings_)
    : BufferWithOwnMemory<SeekableReadBuffer>(buf_size_)
    , log(&Poco::Logger::get("ReadIndirectBufferFromWebServer"))
    , context(context_)
    , url(url_)
    , buf_size(buf_size_)
    , read_settings(settings_)
{
}


std::unique_ptr<ReadBuffer> ReadIndirectBufferFromWebServer::initialize()
{
    Poco::URI uri(url);

    read_settings.http_start_offset = offset;
    LOG_DEBUG(log, "Reading from offset: {}", offset);

    const auto & settings = context->getSettingsRef();
    const auto & config = context->getConfigRef();
    Poco::Timespan http_keep_alive_timeout{config.getUInt("keep_alive_timeout", 20), 0};

    return std::make_unique<ReadWriteBufferFromHTTP>(
        uri,
        Poco::Net::HTTPRequest::HTTP_GET,
        ReadWriteBufferFromHTTP::OutStreamCallback(),
        ConnectionTimeouts(std::max(Poco::Timespan(settings.http_connection_timeout.totalSeconds(), 0), Poco::Timespan(20, 0)),
                           settings.http_send_timeout,
                           std::max(Poco::Timespan(settings.http_receive_timeout.totalSeconds(), 0), Poco::Timespan(20, 0)),
                           settings.tcp_keep_alive_timeout,
                           http_keep_alive_timeout),
        credentials,
        0,
        buf_size,
        read_settings);
}


bool ReadIndirectBufferFromWebServer::nextImpl()
{
    if (impl)
    {
        /// Restore correct position at the needed offset.
        impl->position() = position();
        assert(!impl->hasPendingData());
    }
    else
    {
        impl = initialize();
    }

    auto result = impl->next();
    if (result)
    {
        BufferBase::set(impl->buffer().begin(), impl->buffer().size(), impl->offset());
        offset += working_buffer.size();
    }

    return result;
}


off_t ReadIndirectBufferFromWebServer::seek(off_t offset_, int whence)
{
    if (impl)
        throw Exception(ErrorCodes::CANNOT_SEEK_THROUGH_FILE, "Seek is allowed only before first read attempt from the buffer");

    if (whence != SEEK_SET)
        throw Exception(ErrorCodes::CANNOT_SEEK_THROUGH_FILE, "Only SEEK_SET mode is allowed");

    if (offset_ < 0)
        throw Exception(ErrorCodes::SEEK_POSITION_OUT_OF_BOUND, "Seek position is out of bounds. Offset: {}", std::to_string(offset_));

    offset = offset_;

    return offset;
}


off_t ReadIndirectBufferFromWebServer::getPosition()
{
    return offset - available();
}

}
