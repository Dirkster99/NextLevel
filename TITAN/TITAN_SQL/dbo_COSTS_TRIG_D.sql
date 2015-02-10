
--
-- The COSTS delete trigger is fired when records in the COSTS table are deleted
-- Each deletion of a record changes the performance of a shop, the procedure
-- proc_ShopPerformance does the processing
--
create trigger COSTS_TRIG_D on COSTS for DELETE as
begin

  DECLARE crs_ShopPerform CURSOR FOR -- Get all fields from that table
    SELECT d.COST_ID
          ,d.SHOP_ID
          ,d.PRICE
          ,CUR.NAMEvc
          ,CTP.DESCRIPTIONvc
     FROM deleted d, COST_TYPES CTP, CURRENCY CUR
    WHERE CUR.CURRENCY_ID = d.CURRENCY_ID
      AND d.TYPE_ID       = CTP.TYPE_ID
  FOR READ ONLY

  declare
   @Error_Text      varchar(1024)
  ,@d_COST_ID       int
  ,@d_SHOP_ID       int
  ,@d_DESCRIPTIONvc varchar(64)
  ,@d_PRICE         numeric(10,2)
  ,@d_NAMEvc        varchar(64)

    OPEN crs_ShopPerform
    WHILE (1 = 1)
    BEGIN
      FETCH crs_ShopPerform INTO @d_COST_ID,@d_SHOP_ID,@d_PRICE,@d_NAMEvc,@d_DESCRIPTIONvc
   
      IF (@@sqlstatus <> 0) BREAK -- Break loop once we finished this
   
      exec proc_ShopPerformance 1, 1, @d_COST_ID, @d_PRICE,@d_NAMEvc, @d_DESCRIPTIONvc,@d_SHOP_ID

    END
    CLOSE crs_ShopPerform
    DEALLOCATE CURSOR crs_ShopPerform

end
