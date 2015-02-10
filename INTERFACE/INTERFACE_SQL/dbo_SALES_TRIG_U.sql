
--
-- The sales update trigger is fired when records in the SALES table are updated
-- Each change in a record is processed by the proc_SalesHistory table which in
-- turn is used to analyze changing data over time ...
--
create trigger SALES_TRIG_U on SALES for UPDATE as
begin

  DECLARE crs_Hist CURSOR FOR -- Get all fields from that table
  SELECT i.SALES_ID
        ,i.PURCHASE_ID                     
        ,i.SHOP_ID                         
        ,i.PRICEnc
        ,CUR.NAMEvc
        ,i.UNITSi
        FROM inserted i, deleted d, CURRENCY CUR, PURCHASE PUR, PRODUCT PROD
       WHERE CUR.CURRENCY_ID = i.CURRENCY_ID
         AND PUR.PURCHASE_ID = i.PURCHASE_ID
         AND PROD.PRODUCT_ID = PUR.PRODUCT_ID
         AND PROD.bVALID     = 1               -- We want only valid entries
         AND d.SALES_ID      = i.SALES_ID
         AND( d.PURCHASE_ID                      <> i.PURCHASE_ID                     
           OR d.SHOP_ID                          <> i.SHOP_ID                         
           OR d.PRICEnc                          <> i.PRICEnc                         
           OR d.CURRENCY_ID                      <> i.CURRENCY_ID                     
           OR d.UNITSi                           <> i.UNITSi                          
            )
  FOR READ ONLY
   
  declare
     @Error_Text varchar(1024)
    ,@i_SALES_ID    int
    ,@i_PURCHASE_ID int
    ,@i_SHOP_ID     int
    ,@i_PRICEnc     numeric(10,2)
    ,@NAMEvc        varchar(64)
    ,@i_UNITSi      int
   
    OPEN crs_Hist
    WHILE (1 = 1)
    BEGIN
      FETCH crs_Hist INTO  @i_SALES_ID, @i_PURCHASE_ID, @i_SHOP_ID, @i_PRICEnc, @NAMEvc, @i_UNITSi
   
      IF (@@sqlstatus <> 0) BREAK -- Break loop once we finished this

      exec proc_SalesHistory 3, @i_SALES_ID, @i_PURCHASE_ID, @i_SHOP_ID, @i_PRICEnc, @NAMEvc, @i_UNITSi

      exec proc_ShopPerformance 3, 2, @i_PURCHASE_ID, @i_PRICEnc,@NAMEvc, "",@i_SHOP_ID

    END
    CLOSE crs_Hist
    DEALLOCATE CURSOR crs_Hist

end -- End of Trigger
