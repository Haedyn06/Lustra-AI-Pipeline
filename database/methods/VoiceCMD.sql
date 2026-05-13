DROP TABLE IF EXISTS commandLogs;
CREATE TABLE commandLogs (
    id INT AUTO_INCREMENT PRIMARY KEY,
    command TEXT,
    timestamp DATETIME DEFAULT NOW()
);

DROP TABLE IF EXISTS tempSpeech;
CREATE TABLE tempSpeech (
    tempcommand TEXT
);



DROP PROCEDURE IF EXISTS storeToTempSpeech;
CREATE PROCEDURE storeToTempSpeech(IN cmdSpeech TEXT)
BEGIN
    DELETE FROM tempSpeech;
    INSERT INTO tempSpeech (tempcommand) VALUES (cmdSpeech);
END;

DROP PROCEDURE IF EXISTS storeToCommandLogs;
CREATE PROCEDURE storeToCommandLogs()
BEGIN
    INSERT INTO commandLogs (command)
        SELECT tempcommand FROM tempSpeech
        LIMIT 1;
END;

DROP PROCEDURE IF EXISTS getTempSpeech;
CREATE PROCEDURE getTempSpeech()
BEGIN
    SELECT * FROM tempSpeech;
END;


CALL storeToCommandLogs();
SELECT * FROM commandLogs;

DROP TABLE commandLogs;