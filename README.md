# p8
PostgreSQL shell with UTF-8 coding for Windows  
<b>In the program to execute a command or a query push “Enter” twice.</b><br>
Working with PSQL in Windows I had trouble with code page and diacritic characters.  So I decided to create a small program with my UTF-8 library switching  the console code page to UTF-8.

 To avoid to switch from p8 to psql  and vice-versa when I work I included the most common functionalities. I didn’t’ even try to make these functionalities absolute bug proof. I tested them on my simple database, I would need a database with all features  (schemas , partitions, foreign tables, and so on)  switched to test them . So if your database  uses  more features and the result is not satisfying just change the queries in the program.  
 
