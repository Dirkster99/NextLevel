--
-- The COSTTYPE update trigger is fired when records in the COSTTYPE table are updated
-- Costtypes in existence are not expected to change, if they do, manual work on the
-- database is required to reflect the change, therefor we do error reporting here
-- and the developer/dba is expected to do the rest
--
create trigger COSTTYPE_TRIG_U on COST_TYPES for UPDATE as
begin
  DECLARE crs_COSTTYPE CURSOR FOR -- Get all fields from that table
    SELECT d.TYPE_ID      , i.TYPE_ID                         
          ,d.DESCRIPTIONvc, i.DESCRIPTIONvc                   
      FROM inserted i, deleted d
     WHERE d.TYPE_ID        = i.TYPE_ID
       AND d.DESCRIPTIONvc <> i.DESCRIPTIONvc
  FOR READ ONLY
   
  declare
  @Error_Text varchar(1024)
 ,@d_TYPE_ID       int         , @i_TYPE_ID       int
 ,@d_DESCRIPTIONvc varchar(64) , @i_DESCRIPTIONvc varchar(64)
 
  OPEN crs_COSTTYPE
  WHILE (1 = 1)
  BEGIN
    FETCH crs_COSTTYPE INTO @d_TYPE_ID, @i_TYPE_ID
                           ,@d_DESCRIPTIONvc, @i_DESCRIPTIONvc
 
    IF (@@sqlstatus <> 0) BREAK -- Get out of here if there is no data left to process
 
    select @Error_Text = ' TYPE_ID='       + convert(varchar, @d_TYPE_ID)
                       + ',DESCRIPTIONvc=' + @d_DESCRIPTIONvc + ' => ' + @i_DESCRIPTIONvc
 
    exec proc_Error 'COST_TYPE_TRIG_U', @Error_Text, 'ERROR: Unexpected UPDATE on COST_TYPE!!'
 
  END
  CLOSE crs_COSTTYPE
  DEALLOCATE CURSOR crs_COSTTYPE

end
go
