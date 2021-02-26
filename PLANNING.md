# WCI Points Planning
Planning for WCI Points app. We can change the name of the repo later.

The app needs to account for two main components: students and academics/activities/sports. A table with the attributes and type for each are shown. Even though the items are represented as objects, given the fact that there isn't much manipulation in the app, and how closely it will be tied with the MySQL database, it will probably be easier to simply represent each object as a SQL row and just work with that.

Since the point system and requirements are quite complex, they are excluded from the student table. Also note that the grad year for students should be adjustable, since students can take fifth years.

#### Student
|attribute|type|required|
|---|---|---|
|OEN|string|1|
|Last name|string|1|
|First (legal) name|string|1|
|Preferred name|string|0|
|Start year|int|1|
|Grad year|int|1|


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

#### Sport
|attribute|type|required|
|---|---|---|
|Identifier|int|1|
|Name|string|1|
|Points|int|1|
