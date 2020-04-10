USE hw_mto10_u1750979;

DROP TABLE IF EXISTS student;
DROP TABLE IF EXISTS games;

CREATE TABLE student(
    stu_no CHAR(7) NOT NULL PRIMARY KEY, 
    stu_name CHAR(8) NOT NULL, 
    stu_password CHAR(32) NOT NULL, 
    stu_common_password CHAR(32) NOT NULL, 
    stu_enable CHAR(1) NOT NULL DEFAULT '1' 
);

CREATE TABLE games(
    start_time DATETIME NOT NULL,
    stu_no CHAR(7) NOT NULL,
    stu_name CHAR(8) NOT NULL, 
    mapid CHAR(10) NOT NULL,
    row CHAR(2) NOT NULL,
    col CHAR(2) NOT NULL,
    score CHAR(6) NOT NULL,
    step CHAR(5) NOT NULL,
    max_value CHAR(3) NOT NULL,
    result CHAR(15) NOT NULL,
    avg_score CHAR(8) NOT NULL
);

INSERT INTO student VALUES("1750979","ว๐มุ","07067e648cffc4a7b15857a5da45e238","07067e648cffc4a7b15857a5da45e238","1");


INSERT INTO games VALUES("2020-04-10","21:07:02","1750979","ว๐มุ", "102102866",  "5",  "9", "2239" , "62" , "7" ,"GameOver","49");  
INSERT INTO games VALUES("2020-04-10","21:07:02","1750979","ว๐มุ", "102102866",  "5",  "9", "2239" , "62" , "7" ,"GameOver","49");  
INSERT INTO games VALUES("2020-04-10","21:06:57","1750979","ว๐มุ", "121626310",  "5",  "8", "1684" , "68" , "6" ,"GameOver","42");  
INSERT INTO games VALUES("2020-04-10","21:06:55","1750979","ว๐มุ", "776099987",  "7",  "7", "1867" , "59" , "7" ,"GameOver","38");  
INSERT INTO games VALUES("2020-04-10","21:06:54","1750979","ว๐มุ", "1581499344",  "8",  "7", "2029" , "50" , "9" ,"GameOver","36");  
INSERT INTO games VALUES("2020-04-10","21:06:53","1750979","ว๐มุ", "217274670",  "8",  "8", "2158" , "54" , "10" ,"GameOver","33");  
INSERT INTO games VALUES("2020-04-10","21:06:52","1750979","ว๐มุ", "62261594",  "5",  "6", "1459" , "46" , "7" ,"GameOver","48");  
INSERT INTO games VALUES("2020-04-10","21:06:51","1750979","ว๐มุ", "1957056811",  "5",  "9", "1936" , "50" , "9" ,"GameOver","43");  
INSERT INTO games VALUES("2020-04-10","21:06:50","1750979","ว๐มุ", "1730378820",  "8",  "8", "4708" , "98" , "29" ,"GameOver","73");  
INSERT INTO games VALUES("2020-04-10","21:06:48","1750979","ว๐มุ", "188918927",  "6",  "8", "3091" , "84" , "9" ,"GameOver","64");  
INSERT INTO games VALUES("2020-04-10","21:06:47","1750979","ว๐มุ", "1031150450",  "6",  "10","2602" , "63" , "9" ,"GameOver","43");  
INSERT INTO games VALUES("2020-04-10","21:06:45","1750979","ว๐มุ", "600525769",  "8",  "6", "1816" , "53" , "9" ,"GameOver","37");  
INSERT INTO games VALUES("2020-04-10","21:06:44","1750979","ว๐มุ", "412279130",  "8",  "5", "3298" , "80" , "8" ,"GameOver","82");  
INSERT INTO games VALUES("2020-04-10","21:06:37","1750979","ว๐มุ", "1834193966",  "5",  "5", "1018" , "30" , "8" ,"GameOver","40");  
INSERT INTO games VALUES("2020-04-10","21:06:04","1750979","ว๐มุ", "558245412",  "5",  "5", "1456" , "43" , "8" ,"GameOver","58");  
INSERT INTO games VALUES("2020-04-10","20:33:49","1750979","ว๐มุ", "339586249",  "5",  "5", "598" , "16" , "6" ,"GameOver","23");  
INSERT INTO games VALUES("2020-04-10","20:33:32","1750979","ว๐มุ", "2138406985",  "5",  "5", "1306" , "42" , "7" ,"GameOver","52"); 