
-- proc_Error is called whenever errors need to be locked away
-- in order to have a history of unprocessed events/data or other problems
create procedure proc_Error
(
  @SOURCEvc  VARCHAR(32)
 ,@PARAMSvc  VARCHAR(255)
 ,@MESSAGEvc VARCHAR(255)
)
AS
BEGIN
  INSERT INTO ERROR_MSG(NOWdt, SOURCEvc, PARAMSvc, MESSAGEvc)
  VALUES(getdate(), @SOURCEvc ,@PARAMSvc ,@MESSAGEvc)

END
go
