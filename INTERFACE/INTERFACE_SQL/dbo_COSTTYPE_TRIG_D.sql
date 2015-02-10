
--
-- The COSTTYPE delete trigger is fired when records in the COSTTYPE table are deleted
-- Costtypes in existence are not expected to change, if they do, manual wor on the
-- database is required to reflect the change, therefor we do error reporting here
-- and the developer/dba is expected to do the rest
--
create trigger COSTTYPE_TRIG_D on COST_TYPES for DELETE as
begin

  DECLARE crs_COSTTYPE CURSOR FOR -- Get all fields from that table
    SELECT d.TYPE_ID, d.DESCRIPTIONvc
      FROM deleted d
  FOR READ ONLY
 
  declare
   @Error_Text      varchar(1024)
  ,@d_TYPE_ID       int
  ,@d_DESCRIPTIONvc varchar(64)
 
  OPEN crs_COSTTYPE
  WHILE (1 = 1)
  BEGIN
    FETCH crs_COSTTYPE INTO @d_TYPE_ID, @d_DESCRIPTIONvc
 
    IF (@@sqlstatus <> 0) BREAK -- Get out of here if there is no data left to process
 
    select @Error_Text =  ' TYPE_ID='       + convert(varchar, @d_TYPE_ID)
                        + ',DESCRIPTIONvc=' + convert(varchar, @d_DESCRIPTIONvc)
 
    exec proc_Error 'COST_TYPE_TRIG_D', @Error_Text, 'ERROR: Unexpected DELETE on COST_TYPE!!' 
  END
  CLOSE crs_COSTTYPE
  DEALLOCATE CURSOR crs_COSTTYPE

end
go

