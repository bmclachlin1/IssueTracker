# Main Menu

## Login

```
    )                                         
 ( /(        )    *   )          )         )  
 )\())    ( /(  ` )  /((      ( /(   (  ( /(  
((_)\  (  )\())  ( )(_))\  (  )\()) ))\ )\()) 
 _((_) )\(_))/  (_(_()|(_) )\((_)\ /((_|_))/  
| || |((_) |_   |_   _|(_)((_) |(_|_)) | |_   
| __ / _ \  _|    | |  | / _|| / // -_)|  _|  
|_||_\___/\__|    |_|  |_\__||_\_\\___| \__|  

        Issue Tracking, made spicy

            © Team Jalapeno, 2020
            
  By: Everett Blakley, Blake MemLeaklin, and 
               Steven Trinh
               
Connecting to server at "http://localhost:8080"...

Please enter your name to login: 
```

Make a `GET` request to `http://localhost:8080/users?name=<user name>`

If the `User` exists,  store it and then continue. Else, create a new user by making a `POST` request to `http://localhost:8080/users` with `{"name": "<user name>", "role": "Developer"}` in the request body. Then, assuming that is successful, store the returned `User` and continue. 

Display a welcome message

```
Welcome back, <user name>
```

## Selecting something to do

```
Main Menu
1.	Create a new issue
2.	View open issues
3.	View issues assigned to me
4.	Manage Users
Please select an option (1-4): 
```

# Issues

## Create an Issue

```
New Issue:
Title (max 256 characters): <title>
Description (max 1000 character): <description>
Set the status:
1.	New
2.	Assigned
3.	Fixed
4.	Won't Fix
Please select a status: <status>
Assign to someone? (y/n): <assign>
Select an assignee:
1.	<user 1>
...
Please select a User from the list to assign (1-n): <assignedTo>

Creating our issue...
Issue created successfully!
```

