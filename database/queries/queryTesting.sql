CALL CreateTempLogs(12);
SELECT * FROM `tempChatLogs`;

CALL StoreToUserChat("lamo", "wdada");
SELECT * FROM `tempUserInput`;


SELECT actions FROM aiChatLogs
    WHERE actions IS NOT NULL;