
--
-- The COSTS insert trigger is fired when records in the COSTS table are inserted
-- Each new record changes the performance of a shop, the procedure
-- proc_ShopPerformance does the processing
--
create trigger COSTS_TRIG_I on COSTS for INSERT as
begin
  DECLARE crs_ShopPerform CURSOR FOR -- Get all fields from that table
    SELECT i.COST_ID
          ,i.SHOP_ID
          ,i.PRICE
          ,CUR.NAMEvc
          ,CTP.DESCRIPTIONvc
     FROM inserted i, COST_TYPES CTP, CURRENCY CUR
    WHERE CUR.CURRENCY_ID = i.CURRENCY_ID
      AND i.TYPE_ID = CTP.TYPE_ID
  FOR READ ONLY

  declare
   @Error_Text varchar(1024)
  ,@i_COST_ID     int
  ,@i_SHOP_ID     int
  ,@i_DESCRIPTIONvc varchar(64)
  ,@i_PRICE       numeric(10,2)
  ,@i_NAMEvc        varchar(64)

    OPEN crs_ShopPerform
    WHILE (1 = 1)
    BEGIN
      FETCH crs_ShopPerform INTO @i_COST_ID, @i_SHOP_ID, @i_PRICE, @i_NAMEvc, @i_DESCRIPTIONvc
   
      IF (@@sqlstatus <> 0) BREAK -- Break loop once we finished this
   
      exec proc_ShopPerformance 2, 1, @i_COST_ID, @i_PRICE,@i_NAMEvc, @i_DESCRIPTIONvc,@i_SHOP_ID
   
    END
    CLOSE crs_ShopPerform
    DEALLOCATE CURSOR crs_ShopPerform

end
