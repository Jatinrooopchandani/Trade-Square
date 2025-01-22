Objective:
The objective of this project is to create a program that helps a seller identify the most
profitable stores to sell products to, considering both profit potential and deadlines for
delivery.
![image](https://github.com/user-attachments/assets/707314bf-de4c-4ac6-9f00-90439c8df14e)
![image](https://github.com/user-attachments/assets/3b4779df-699d-4583-b817-65bc27bf0610)

How to run:
To run this project you must download and set up SDL2 and SDL2_ttf in your system. Here is a tutorial link: https://www.youtube.com/watch?v=tmGBhM8AEj8&feature=youtu.be



Functionality:
The program allows the user to:
Add a new store with its name, coordinates, profit, and deadline.
Delete an existing store by its name.
View all stores’ data stored in a file.
Identify the most profitable stores to sell to, considering their distances from the
seller's location and their profit potential.
Display the order in which the selected stores should be visited based on scheduling
to maximize profit.

Data Representation:
Stores are represented as objects of the Store struct, containing attributes such as
coordinates (x, y), profit, deadline, and name.
Store data is stored in a file named "stores.txt".
Functionality Implementation:
Adding a new store appends the store data to the "stores.txt" file.
Deleting a store removes its data from the file.
Fetching store data reads from the file and populates a vector of Store objects.
Calculating the closest stores utilizes a distance calculation function to find the
Euclidean distance between the seller and each store, and then selects the closest
stores based on this distance.
Scheduling jobs sorts the selected stores based on profit and deadlines to determine
the order in which they should be visited.

Visualization:
The program uses SDL (Simple DirectMedia Layer) for graphics rendering.
It displays a city map image with marked store locations and the seller's location.
Stores are represented as filled circles on the map, with their names displayed above
them.
The seller's location is marked with a red filled circle labeled as "Seller".
During operation, the program gradually highlights the selected stores with blue
circles to visualize the process
