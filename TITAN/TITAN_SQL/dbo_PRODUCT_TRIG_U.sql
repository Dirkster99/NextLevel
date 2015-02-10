
--
-- The PRODUCT update trigger is fired when records in the PRODUCT table are updated
-- Each change in a record is processed by the proc_SalesHistory table which in
-- turn is used to analyze changing data over time ...
--
create trigger PRODUCT_TRIG_U on PRODUCT for UPDATE as
begin

  DECLARE crs_Hist CURSOR FOR -- Get all fields from that table
  SELECT SALE.SALES_ID
        ,SALE.PURCHASE_ID                     
        ,SALE.SHOP_ID                         
        ,SALE.PRICEnc
        ,CUR.NAMEvc
        ,SALE.UNITSi
        FROM inserted i, deleted d, CURRENCY CUR, PURCHASE PUR, SALES SALE
       WHERE d.PRODUCT_ID    = i.PRODUCT_ID
         AND i.PRODUCT_ID    = PUR.PRODUCT_ID
         AND PUR.PURCHASE_ID = SALE.PURCHASE_ID
         AND CUR.CURRENCY_ID = SALE.CURRENCY_ID
         AND i.bVALID       <> d.bVALID          -- We want only valid entries, so we react
  FOR READ ONLY                                 -- if and when this changes
   
  declare
     @SALES_ID    int
    ,@PURCHASE_ID int
    ,@SHOP_ID     int
    ,@PRICEnc     numeric(10,2)
    ,@NAMEvc        varchar(64)
    ,@UNITSi      int
   
    OPEN crs_Hist
    WHILE (1 = 1)
    BEGIN
      FETCH crs_Hist INTO  @SALES_ID, @PURCHASE_ID, @SHOP_ID, @PRICEnc, @NAMEvc, @UNITSi
   
      IF (@@sqlstatus <> 0) BREAK -- Break loop once we finished this

      exec proc_SalesHistory 3, @SALES_ID, @PURCHASE_ID, @SHOP_ID, @PRICEnc, @NAMEvc, @UNITSi
    END
    CLOSE crs_Hist
    DEALLOCATE CURSOR crs_Hist

  UPDATE PRODUCT_PRICE
     SET Product_ID         = i.PRODUCT_ID
        ,NAMEvc             = i.NAMEvc
        ,DESCRIPTIONvc      = i.DESCRIPTIONvc
        ,bVALID             = i.bVALID
        ,STDRD_PRICE_ID     = STD_PR.PRICE_ID     -- STDRD Records
        ,STDRD_VALID_FROMdt = STD_PR.VALID_FROMdt
        ,STDRD_VALID_TOdt   = STD_PR.VALID_TOdt
        ,STDRD_PRICEnc      = STD_PR.PRICEnc
        ,STDRD_CURRENCY_ID  = STD_PR.CURRENCY_ID
        ,SALES_PRICE_ID      = SLPR.PRICE_ID      -- SALES Records
        ,SALES_VALID_FROMdt  = SLPR.VALID_FROMdt
        ,SALES_VALID_TOdt    = SLPR.VALID_TOdt
        ,SALES_PRICEnc       = SLPR.PRICEnc
        ,SALES_CURRENCY_ID   = SLPR.CURRENCY_ID
        ,SALES_DESCRIPTIONvc = SLPR.DESCRIPTIONvc
        ,SALES_NAMEvc        = SLPR.NAMEvc
        ,CAT_NAMEvc          = CAT.NAMEvc       --CATEGORY_1_ID    int         not null,
        ,CAT_SUB_NAMEvc      = CAT.SUB_NAMEvc   --CATEGORY_2_ID    int         not null
        ,CAT_DESCRIPTIONvc   = CAT.SUB_NAMEvc
    FROM inserted i, deleted d, STANDARDPRICE STD_PR, SALESPRICE SLPR, CATEGORY CAT
   WHERE d.PRODUCT_ID    = i.PRODUCT_ID
     AND STD_PR.PRICE_ID = i.STDRD_PRICE_ID
     AND SLPR.PRICE_ID   = i.SALES_PRICE_ID
     AND CAT.CAT_1_ID    = i.CATEGORY_1_ID
     AND CAT.CAT_1_ID    = i.CATEGORY_2_ID

end -- End of Trigger
