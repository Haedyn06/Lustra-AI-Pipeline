DESC aiChatLogs;

ALTER TABLE aiChatLogs
    ADD source VARCHAR(15); 

SELECT * FROM aiChatLogs;
SELECT * FROM mqttLogs;

CALL StoreToChat("joemam")
CALL RemoveFromChat(1);


ALTER TABLE aiChatLogs
    add actions TEXT;

DESC aiChatLogs;

CALL GetRecentMsgIII(1);
