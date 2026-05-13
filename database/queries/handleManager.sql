---------------------------
-- CREATE TABLE CHATLOGS --
---------------------------
CALL CreateChatLogs(10);

SELECT * FROM `aiChatLogs`;
SELECT * FROM `tempChatLogs`;

-----------------------
-- INSERT TEMP TABLE --
-----------------------

DROP TABLE IF EXISTS `tempChatLogs`;

CREATE TABLE tempChatLogs (
    tempOrdered INT AUTO_INCREMENT PRIMARY KEY,
    tempUserMsg TEXT, 
    tempAiMsg TEXT,  
    TempMsgDate DATETIME DEFAULT CURRENT_TIMESTAMP
);

INSERT INTO tempChatLogs (tempUserMsg, tempAiMsg, TempMsgDate)
    SELECT userMsg, aiMsg, msgDate
        FROM (
            SELECT userMsg, aiMsg, msgDate FROM aiChatLogs
            ORDER BY ordered DESC
            LIMIT 10
        ) AS recentMessages
    ORDER BY msgDate;

SELECT * FROM `tempChatLogs`;

------------------------
-- CHATLOG MANAGEMENT --
------------------------

SELECT * FROM `tempAIResponse`;
DROP TABLE tempUserInput;
SELECT * FROM `tempUserInput`;

SELECT * FROM `aiChatLogs`;
SELECT * FROM `tempChatLogs`;
SHOW ENGINE INNODB STATUS;
SHOW PROCESSLIST;


CALL `CleanChat`(12); -- New AI CHAT
CALL `RemoveFromChat`(6); -- Remove last messages
CALL `AddToChat`("Hello", "Hi There"); -- Add chatlog

CALL `GetRecentMsg`(1);
CALL `GetRecentMsgIII`(1);
CALL `StoreToChat`();
CALL GetInput();
CALL `StoreToAIChat`("");
CALL StoreToUserChat("");
CALL `DeleteSomeChats`(1);


SHOW FULL PROCESSLIST;

