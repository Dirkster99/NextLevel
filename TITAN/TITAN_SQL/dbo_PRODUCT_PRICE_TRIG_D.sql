
--
-- The PRODUCT_PRICE tables is a decomposition of the
-- product, StandardPrice, SalesPrice, and Category table
-- Normally, references to products are never deleted,
-- but never say never, so if a product reference is deleted an error
-- message is stored and the dba/developer is expected to patch things manually...
--
create trigger PRODUCT_PRICE_TRIG_D on PRODUCT_PRICE for DELETE as
begin

DECLARE cu_all CURSOR FOR -- Get all fields from that table
  SELECT d.Product_ID
        ,d.NAMEvc
        ,d.DESCRIPTIONvc
        ,d.bValid
    FROM deleted d
FOR READ ONLY
 
declare
   @PARAMSvc        varchar(1024)
  ,@d_Product_ID    int
  ,@d_NAMEvc        varchar(64)
  ,@d_DESCRIPTIONvc varchar(255)
  ,@d_bValid        bit
  
  OPEN cu_all
  WHILE (1 = 1)
  BEGIN
    FETCH cu_all INTO @d_Product_ID ,@d_NAMEvc ,@d_DESCRIPTIONvc ,@d_bValid
 
    IF (@@sqlstatus <> 0) BREAK
 
    select @PARAMSvc = 
        ' Product_ID=' + convert(varchar, @d_Product_ID)
      + ',NAMEvc=' + convert(varchar, @d_NAMEvc)
      + ',DESCRIPTIONvc=' + convert(varchar, @d_DESCRIPTIONvc)
      + ',bValid=' + convert(varchar, @d_bValid)

    exec proc_Error "PRODUCT_PRICE_TRIG_D", @PARAMSvc, "ERROR: PRODUCT ENTRY PHYSICALLY DELETED!"
  END
  CLOSE cu_all
  DEALLOCATE CURSOR cu_all
end -- End of Trigger
