-- { echoOn }
-- simple test case
create table if not exists replacing_mt (x String) engine=ReplacingMergeTree() ORDER BY x SETTINGS force_select_final=1;

insert into replacing_mt values ('abc');
insert into replacing_mt values ('abc');

-- expected output is 1 because force_select_final is turned on
select count() from replacing_mt;

-- JOIN test cases
create table if not exists lhs (x String) engine=ReplacingMergeTree() ORDER BY x SETTINGS force_select_final=1;
create table if not exists rhs (x String) engine=ReplacingMergeTree() ORDER BY x SETTINGS force_select_final=1;

insert into lhs values ('abc');
insert into lhs values ('abc');

insert into rhs values ('abc');
insert into rhs values ('abc');

-- expected output is 1 because both tables have force_select_final = 1
select count() from lhs inner join rhs on lhs.x = rhs.x;

drop table lhs;
drop table rhs;

create table if not exists lhs (x String) engine=ReplacingMergeTree() ORDER BY x;
create table if not exists rhs (x String) engine=ReplacingMergeTree() ORDER BY x SETTINGS force_select_final=1;

insert into lhs values ('abc');
insert into lhs values ('abc');

insert into rhs values ('abc');
insert into rhs values ('abc');

-- expected output is 2 because lhs table doesn't have final applied
select count() from lhs inner join rhs on lhs.x = rhs.x;
