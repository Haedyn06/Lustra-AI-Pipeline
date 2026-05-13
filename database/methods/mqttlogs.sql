CREATE TABLE mqttLogs (
    ordered INT AUTO_INCREMENT PRIMARY KEY,
    topicReceived TEXT, 
    msgReceived TEXT,  
    msgDate DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE mqttTopicCompare (
    ordered INT AUTO_INCREMENT PRIMARY KEY,
    topicReceived TEXT, 
    topicSend TEXT 
);

INSERT INTO mqttTopicCompare (topicReceived, topicSend) VALUES ("lustrasim/lustratalk/input", "lustrasim/lustratalk/response");


DROP PROCEDURE IF EXISTS recentMqttLog;
CREATE PROCEDURE recentMqttLog(IN amount INT)
BEGIN
    SELECT topic, msg FROM mqttLogs
    ORDER BY msgDate DESC, ordered DESC
    LIMIT amount;
    -- Go to cmqttTopicCompare table
    -- Check matching topicReceive
    -- If match
    -- Return the topicSend
END $$

DROP PROCEDURE IF EXISTS recentMqttLog;
DELIMITER $$

CREATE PROCEDURE recentMqttLog(IN amount INT)
BEGIN
    SELECT 
        l.topicReceived AS topic,
        l.msgReceived AS message,
        c.topicSend AS matchedTopicSend
    FROM mqttLogs l
    LEFT JOIN mqttTopicCompare c
        ON l.topicReceived = c.topicReceived
    ORDER BY l.msgDate DESC, l.ordered DESC
    LIMIT amount;
END $$

DELIMITER ;


SELECT * FROM mqttLogs;
SELECT * FROM mqttTopicCompare;


CREATE PROCEDURE recentMqttLog()
BEGIN
    SELECT topicReceived 
    FROM mqttLogs
    ORDER BY msgDate DESC, ordered DESC
    LIMIT 1;
END;
DROP PROCEDURE IF EXISTS recentMqttLog;
DROP TABLE mqttLogs;
DROP TABLE mqttTopicCompare;