# Tyr Documentation

The app needs to account for three main components: students, actions (academics/activities/athletics), and awards.  
Each one of these components are discussed in terms of objects, but since Tyr is so closely tied with the MySQL database, they are dealt with as SQL rows in the app.

## Students
Each student will have an Ontario Education Number, a name, and a start and graduation year.  
The start year represents the very start of a student's high school career. For example, if a student enters high school in the 2019-2020 school year, the start year should be 2019.  
The grad year represents the very end of a student's high school career. For example, if a student graduates high school in the 2022-2023 school year, the grad year should be 2023.  
Note that the grad year for students should be adjustable, since students can take fifth years or graduate early.

A student schematic is given below:

#### Student
|attribute|type|required|
|---|---|---|
|Identifier|int|1|
|OEN|string|1|
|Last name|string|1|
|First (legal) name|string|1|
|Preferred name|string|0|
|Start year|year|1|
|Grad year|year|1|

## Actions
Students can achieve three types of _actions_ for points of that type. E.g, they can participate in an athletic to get athletic points.
Awards can be based on these points, but may also be based on having completed certain actions (see Awards section).

Each action also has a type, being 0 for an Athletic, 1 for an Academic, and 2 for an Activity. All types of actions are storeds in a single table for easier SQL queries.

An action can give a different amount of points (e.g., major sports are worth 2 points while minor ones are worth only 1).
Rather than keeping track of which students have however many points, it's better to just calculate it on the fly with a SQL SUM() function when assigning awards.

#### Action
|attribute|type|required|
|---|---|---|
|Identifier|int|1|
|Type|digit|1
|Name|string|1|
|Points|int|1|

A linking tables is used to model a student completing an action in a particular school year, being a many-to-many relationship. The starting year of the school year is used.
For example, if a student participated in a club in the 2020-2021 school year, the start year should be 2020.

The linking table schematic is given below.

#### Student x Action
|attribute|type|required|
|---|---|---|
|Student ID|int|1|
|Action ID|int|1|
|Start year|year|1|

Since past actions may not always apply, there should be the option to remove actions.
However, directly removing them from the database would cause all past students who took that action to be affected (and wouldn't be possible since foreign keys are enforced).
Instead, an archived action table stores old actions that are no longer displayed in the UI, but that are kept for compatibility.
In the case that there are no references to the action, removing it should delete it from the database.

#### Action Archive
|attribute|type|required|
|---|---|---|
|Identifier|int|1|

## Awards
Each award can be represented as a name and one or more sets of constraints. A constraint is a hypothetical object that evaluates to true if all of its conditions evaluate to true.
A student who satisfies any set of those constraints will be granted that award.
These constraints can be quite complex, involving multiple conditions and sometimes even requiring other awards as constraints.
The constraint sets of the base awards are given below, but the application should easily be extendable to create new awards.

The Athletic Red W award has two constraint sets:
1. 12 Athletic points
2. 4 years of the same Athletic action

Note that the Red W award is satisfied if EITHER constraint set 1 OR 2 is satisfied.

The Academic White W award has three constraint sets:
1. <= 4 years of high school AND 3 or more years of High Honours
2. \> 4 years of high school AND 4 or more years of High Honours
3. \> 4 years of high school AND 3 consecutive years of High Honours

Note that to satisfy a single constraint set, ALL constraints in ONE SET must be satisfied. Note also that High Honours is a specific Academic action.

The Activity Blue W award has two constraint sets:
1. <= 4 years of high school AND 8 Activity points with at most 3 points per year
2. \> 4 years of high school AND 10 Activity points with at most 3 points per year

Note that a cap is placed on the number of points each year.

The Award of Honour has one single constraint set:
1. Red W AND White W AND Blue W

Note that it treats the Red W, White W,and Blue W as constraints.

There are two issues to deal with here: complex constraints with many requirements and treating awards as constraints.

### Awards as Constraints
In order for the app to be able to check to see if the Award of Honour is satisfied, the awards it's based on must be decomposed into constraints.
One way to do this is to directly use the constraint sets of the Red W, White W, and Blue W.
However, since only one of their constraint sets need to be satisfied for the entire award to be satisfied, we'd need to check it against every single combination of their constraint sets.
That means the Award of Honour would need 2\*3*2=12 constraint sets. If we were to add more awards with their own constraint sets, the time and space complexity would grow exponentially.

Instead, we can treat awards and constraints as the same thing (an award essentially inherits from a constraint). This means to check the Award of Honour, we evaluate each W award seperately,
for 2+3+2=7 constraints checked. This reduces complexity to linear, and also allows short circuiting if any required constraint evaluates to false.

The constraints should then form a directed acyclic graph (DAG), and evaluating them will use an order similar to a topological search.

### Complex Constraints
Some constraints are simple, such as "have at least 12 athletic points". However, many constraints are composed of multiple conditionals (or maybe constraints themselves).
Allowing awards to be constraints adds another layer of complexity, since awards and constraints have different behaviour:
a constraint has multiple conditions (some of which themselves can be constraints) that must ALL evaluate to true,
while an award has multiple sets of constraints where only one set needs to evaluate to true.

Thus we can define two kinds of constraints: an "or" constraint that evaluates to the logical _or_ of its sub-constraints, and an "and" constraint that evaluates to the logical _and_.
These kinds of constraints are referred to as _compound_ constraints.

There is also one more constraint we need to define: basic conditionals. A basic conditional is just a SQL query that should return true or false.
There are 4 kinds of basic conditional that will be used in this application:
1. The sum of action points, with at most _m_ points per year, should be in an interval _x_ to _y_
2. The difference between student grad year and start year should be in an interval _x_ to _y_
3.  
	a. The frequency of a specific action should be in an interval _x_ to _y_  
	b. The frequency of _any_ specific action in a specific action type should be in an interval _x_ to _y_  
	c. The frequency of _all_ actions in a specific action type should be in an interval _x_ to _y_  
4.  
	a. The maximum consecutive years of a specific action should be in an interval _x_ to _y_  
	b. The maximum consecutive years of _any_ specific action in a specific action type should be in an interval _x_ to _y_  
	c. The maximum consecutive years of _all_ actions in a specific action type should be in an interval _x_ to _y_  

These calculations are facilitated by SQL procedures. Note that there can be multiple specific conditions that fall under one kind, depending on what information is provided.
Since 3.b and 4.b can have multiple values, we choose the action that maximizes the value. See the SQL schema file for specific implementation.

### Schematic
In theory, each constraint forms the node of a DAG. Since there's not really a sense of a parent/child relationship in a DAG, we refer to it as a super/sub relationship.
Each constraint can use multiple _sub-constraints_, and be used in a _super-constraint_. Note that some aspects are in reverse to a traditional parent/child relationship.

Each of the five types of constraint above theoretically inherits from a constraint abstract class.
In the database, there is a base constraint table with information universal to all three types, and other information is stored on seperate tables.
A constraint has an identifier, a name, a description, a type, and a field marking it as an award or not.

To process all awards, we use a topological sorting of all constraints, where each sub-constraint should be processed before a super-constraint.
This allows us to process all constraints bottom-up in O(n) time, compared to O(2^n) time for a naive top-down processing.

The name field is only required if the constraint is an award. The description field describes the constraint and is used in the UI.  
If the type is 2, 3, 4, or 5, a constraint is basic (corresponds to 1...4 above), and parameter information for a procedure call is stored in a seperate table.
If the type is 0 or 1, it is an "or constraint" or an "and constraint", respectively, and parent-child constraint relationships are stored in a seperate table.

#### Constraint
|attribute|type|required|
|---|---|---|
|Identifier|int|1|
|Name|string|0|
|Description|string|1|
|Type|digit|1|
|Is award|bool|1|

Below, constraint id references the constraint, max points represents _m_ from basic constraint type 1, and interval start/end represent _x_ and _y_.
Action type will be 0 to represent Athletic, 1 for Academic, and 2 for Activity.

#### Basic Constraint
|attribute|type|required|
|---|---|---|
|Constraint ID|int|1|
|Action ID|int|0|
|Action type|digit|0|
|Max points|int|0|
|Interval start|int|1|
|Interval end|int|1|

Below, each super/sub relationship is stored for compound constraints (essentially an edge list).

#### Compound Constraint
|attribute|type|required|
|---|---|---|
|Sub constraint ID|int|1|
|Super constraint ID|int|1|
