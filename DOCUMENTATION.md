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

## Action
Students can achieve three types of _actions_ for points of that type. E.g, they can participate in an athletic to get athletic points. Awards can be based on these points, but may also be based on having completed certain actions (see Awards section).

The different action schematics are given below. An action can give a different amount of points (e.g., major sports are worth 2 points while minor ones are worth only 1).
Rather than keeping track of which students have however many points, it's better to just calculate it on the fly with a SQL SUM() function when assigning awards.

#### Athletic
|attribute|type|required|
|---|---|---|
|Identifier|int|1|
|Name|string|1|
|Points|int|1|

#### Academic
|attribute|type|required|
|---|---|---|
|Identifier|int|1|
|Name|string|1|
|Points|int|1|

#### Activity
|attribute|type|required|
|---|---|---|
|Identifier|int|1|
|Name|string|1|
|Points|int|1|

Linking tables are used to model a student completing an action in a particular school year. The starting year of the school year is used.
For example, if a student participated in a club in the 2020-2021 school year, the start year should be 2020.

The linking table schematics are given below.

#### Student x Athletic
|attribute|type|required|
|---|---|---|
|Student ID|int|1|
|Athletic ID|int|1|
|Start year|year|1|

#### Student x Academic
|attribute|type|required|
|---|---|---|
|Student ID|int|1|
|Academic ID|int|1|
|Start year|year|1|

#### Student x Activity
|attribute|type|required|
|---|---|---|
|Student ID|int|1|
|Activity ID|int|1|
|Start year|year|1|

## Awards
Each award can be represented as a name and one or more sets of constraints. A constraint is a hypothetical object that evaluates to true if all of its conditions evaluate to true.
A student who satisfies any set of those constraints will be granted that award. These constraints can be quite complex, involving multiple conditions and sometimes even requiring other awards as constraints.
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
In order for the app to be able to check to see if the Award of Honour is satisfied, the awards it's based on must be decomposed into constraints. We can use the constraints of the Red W, White W, and Blue W.
However, since only one of their constraint sets need to be satisfied for the entire award to be satisfied, we'd need to check it against every single combination of their constraint sets.
That means the Award of Honour would need 2\*3*2=12 constraint sets. If we were to add more awards with their own constraint sets, the time and space complexity would grow exponentially.

Instead, we can treat awards and constraints as the same thing (an award essentially inherits from a constraint). This means to check the Award of Honour, we evaluate each W award seperately,
for 2+3+2=7 constraints checked. This reduces complexity to linear, and also allows short circuiting if any required constraint evaluates to false.

The constraints should then form a directed acyclic graph, and evaluating them will use an order similar to a topological search.

### Complex Constraints
Some constraints are simple, such as "have at least 12 athletic points". However, many constraints are composed of multiple conditionals (or maybe constraints themselves).
Allowing awards to be constraints adds another layer of complexity, since awards and constraints have different behaviour:
a constraint has multiple conditions (some of which themselves can be constraints) that must ALL evaluate to true,
while an award has multiple sets of constraints where only one set needs to evaluate to true.

Thus we can define two kinds of constraints: an "or" constraint that evaluates to the logical _or_ of its sub-constraints, and an "and" constraint that evaluates to the logical _and_.
There is also one more constraint we need to define: basic conditionals.
