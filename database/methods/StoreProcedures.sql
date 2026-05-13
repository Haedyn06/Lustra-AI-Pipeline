                    -------------------
                    -- CREATE TABLES --
                    -------------------
-- Create ChatLogs
DROP PROCEDURE IF EXISTS CreateChatLogs;
CREATE PROCEDURE CreateChatLogs(IN lim INT)
BEGIN
    DROP TABLE IF EXISTS aiChatLogs;

    CREATE TABLE aiChatLogs (
        ordered INT AUTO_INCREMENT PRIMARY KEY,
        userMsg TEXT,
        aiMsg TEXT,  
        actions TEXT,
        source VARCHAR(15),
        msgDate DATETIME DEFAULT CURRENT_TIMESTAMP
    );

    INSERT INTO aiChatLogs (userMsg, aiMsg, actions, source, msgDate)
        SELECT tempUserMsg, tempaiMsg, tempactions, tempsource, tempmsgDate
        FROM (
            SELECT userMsg, aiMsg, actions, source, msgDate FROM tempChatLogs
            ORDER BY tempordered DESC
            LIMIT lim
        ) AS lastMessages
        ORDER BY tempmsgDate;
END;

-- Create Temp Chat logs
DROP PROCEDURE IF EXISTS CreateTempLogs;
CREATE PROCEDURE CreateTempLogs(IN lim INT)
BEGIN
    DROP TABLE IF EXISTS `tempChatLogs`;

    CREATE TABLE tempChatLogs (
        tempordered INT AUTO_INCREMENT PRIMARY KEY,
        tempuserMsg TEXT,
        tempaiMsg TEXT,  
        tempactions TEXT,
        tempsource VARCHAR(15),
        tempmsgDate DATETIME DEFAULT CURRENT_TIMESTAMP
    );

    INSERT INTO tempChatLogs (tempUserMsg, tempaiMsg, tempactions, tempsource, tempmsgDate)
        SELECT userMsg, aiMsg, actions, source, msgDate
        FROM (
            SELECT userMsg, aiMsg, actions, source, msgDate FROM aiChatLogs
            ORDER BY ordered DESC
            LIMIT lim
        ) AS recentMessages
        ORDER BY msgDate;
END;



-- Create Temp Input
DROP PROCEDURE IF EXISTS CreateTempInput;
CREATE PROCEDURE CreateTempInput()
BEGIN
    CREATE TABLE tempUserInput (
        id INT,
        userMsg TEXT,
        inputSource VARCHAR(15),
        msgDate DATETIME DEFAULT CURRENT_TIMESTAMP
    );
END;
CALL CreateTempInput();

-- Create Temp Response
DROP PROCEDURE IF EXISTS CreateTempResponse;
CREATE PROCEDURE CreateTempResponse()
BEGIN
    CREATE TABLE tempAIResponse (
        id INT AUTO_INCREMENT,
        aiMsg TEXT,
        msgDate DATETIME DEFAULT CURRENT_TIMESTAMP
    );
END;



                        ------------------
                        -- GET MESSAGES --
                        ------------------
DELIMITER ;

DROP PROCEDURE IF EXISTS GetRecentMsg;
CREATE PROCEDURE GetRecentMsg(IN amount INT)
BEGIN
    SELECT userMsg, aiMsg FROM aiChatLogs
    ORDER BY msgDate DESC, ordered DESC
    LIMIT amount;
END;

DROP PROCEDURE IF EXISTS GetRecentMsgII;
CREATE PROCEDURE GetRecentMsgII(IN amount INT)
BEGIN
    SELECT userMsg, aiMsg, msgDate FROM aiChatLogs
    ORDER BY msgDate DESC, ordered DESC
    LIMIT amount;
END;

DROP PROCEDURE IF EXISTS GetRecentMsgIII;
CREATE PROCEDURE GetRecentMsgIII(IN amount INT)
BEGIN
    SELECT * FROM aiChatLogs
    ORDER BY msgDate DESC, ordered DESC
    LIMIT amount;
END;



DROP PROCEDURE IF EXISTS GetInput;
CREATE PROCEDURE GetInput()
BEGIN
    SELECT userMsg FROM tempUserInput ORDER BY id DESC LIMIT 1;
END ;

DROP PROCEDURE IF EXISTS GetInputSource;
CREATE PROCEDURE GetInputSource()
BEGIN
    SELECT inputSource FROM tempUserInput ORDER BY id DESC LIMIT 1;
END ;
CALL GetInputSource();

DROP PROCEDURE IF EXISTS GetResponse;
CREATE PROCEDURE GetResponse()
BEGIN
    SELECT aiMsg FROM tempAIResponse ORDER BY id DESC LIMIT 1;
END ;
CALL GetResponse();

                        --------------------
                        -- STORE MESSAGES --
                        --------------------

DROP PROCEDURE IF EXISTS StoreToUserChat;
CREATE PROCEDURE StoreToUserChat(IN msgFromUser TEXT, IN inputSource TEXT)
BEGIN
DELETE FROM tempUserInput;
INSERT INTO tempUserInput (id, userMsg, inputSource, `msgDate`) VALUES (1, msgFromUser, inputSource, NOW());
END ;

DROP PROCEDURE IF EXISTS StoreToAIChat;
CREATE PROCEDURE StoreToAIChat(IN msgFromAI TEXT)
BEGIN
    IF EXISTS (SELECT 1 FROM tempAIResponse) THEN
        UPDATE tempAIResponse SET aiMsg = msgFromAI ORDER BY id DESC LIMIT 1;
    ELSE
        INSERT INTO tempAIResponse (aiMsg) VALUES (msgFromAI);
    END IF;
END ;

DROP PROCEDURE IF EXISTS StoreToChat;
CREATE PROCEDURE StoreToChat(IN actionType TEXT)
BEGIN
    DECLARE userMessage TEXT;
    DECLARE userSource VARCHAR(15);
    DECLARE aiResponse TEXT;
    
    IF (SELECT COUNT(*) FROM tempUserInput) > 0 AND (SELECT COUNT(*) FROM tempAIResponse) > 0 THEN
        SELECT userMsg INTO userMessage FROM tempUserInput ORDER BY id DESC LIMIT 1;
        SELECT inputSource INTO userSource FROM tempUserInput ORDER BY id DESC LIMIT 1;
        SELECT aiMsg INTO aiResponse FROM tempAIResponse ORDER BY id DESC LIMIT 1;
        INSERT INTO aiChatLogs (userMsg, aiMsg, actions, source, `msgDate`) VALUES (userMessage, aiResponse, actionType, userSource, NOW());
    END IF;
END;

                        ---------------------
                        -- MANAGE MESSAGES --
                        ---------------------
DELIMITER ;

DROP PROCEDURE IF EXISTS RemoveFromChat;
CREATE PROCEDURE RemoveFromChat(IN NUM INT)
BEGIN
    DELETE t1
    FROM aiChatLogs t1
    JOIN (
        SELECT ordered
        FROM aiChatLogs
        ORDER BY ordered DESC
        LIMIT NUM
    ) t2 ON t1.ordered = t2.ordered;
END ;

DROP PROCEDURE IF EXISTS CleanChat;
CREATE PROCEDURE CleanChat(IN lim INT)
BEGIN
    CALL CreateTempLogs(lim);
    CALL CreateChatLogs(lim);
END ;

DROP PROCEDURE IF EXISTS AddToChat;
CREATE PROCEDURE AddToChat(IN msgFromUser TEXT, IN msgFromAI TEXT)
BEGIN
    CALL StoreToUserChat(msgFromUser);
    CALL StoreToAIChat(msgFromAI);
    CALL StoreToChat();
END ;

DELIMITER ;
