
--
-- The sales insert trigger is fired when records in the SALES table are inserted
-- Each new record is processed by the proc_SalesHistory table which in turn is used
-- to analyze changing data over time ...
--
create trigger SALES_TRIG_I on SALES for INSERT as
begin

  declare @CntShop int

  -- This is just another statemant showing how access to an external DB can be implemented
   select @CntShop = count(*)
     from HERCULES.dbo.SHOPS

  DECLARE crs_Hist CURSOR FOR -- Get all fields from that table
  SELECT i.SALES_ID
        ,i.PURCHASE_ID
        ,i.SHOP_ID
        ,i.PRICEnc
        ,CUR.NAMEvc
        ,i.UNITSi
        FROM inserted i, CURRENCY CUR, PURCHASE PUR, PRODUCT PROD
       WHERE CUR.CURRENCY_ID = i.CURRENCY_ID
         AND PUR.PURCHASE_ID = i.PURCHASE_ID
         AND PROD.PRODUCT_ID = PUR.PRODUCT_ID
         AND PROD.bVALID     = 1               -- We want only valid entries
  FOR READ ONLY
 
  declare @i_SALES_ID     int
         ,@i_PURCHASE_ID  int
         ,@i_SHOP_ID      int
         ,@i_PRICEnc      numeric(10,2)
         ,@NAMEvc        varchar(64)
         ,@i_UNITSi       int
 
  OPEN crs_Hist
  WHILE (1 = 1)
  BEGIN
    FETCH crs_Hist INTO @i_SALES_ID,@i_PURCHASE_ID ,@i_SHOP_ID ,@i_PRICEnc ,@NAMEvc ,@i_UNITSi
 
    IF (@@sqlstatus <> 0) BREAK -- Break loop once we finished this
 
    exec proc_SalesHistory 2, @i_SALES_ID, @i_PURCHASE_ID, @i_SHOP_ID, @i_PRICEnc, @NAMEvc, @i_UNITSi

    exec proc_ShopPerformance 2, @i_PURCHASE_ID, @i_PRICEnc,@NAMEvc, "",@i_SHOP_ID
  END
  CLOSE crs_Hist
  DEALLOCATE CURSOR crs_Hist

end -- End of Trigger
