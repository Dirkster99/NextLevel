

--
-- The sales delete trigger is fired when records in the SALES table are deleted
-- Each deletion of a record closes the history of that sales entry, therefore,
-- a record in the sales history is never deleted but simply closed
-- see proc_SalesHistory for more detail
--
create trigger SALES_TRIG_D on SALES for DELETE as
begin

DECLARE crs_Hist CURSOR FOR -- Get all fields from that table
SELECT d.SALES_ID
      ,d.PURCHASE_ID
      ,d.SHOP_ID
      ,d.PRICEnc
      ,CUR.NAMEvc
      ,d.UNITSi
  FROM deleted d, CURRENCY CUR, PURCHASE PUR, PRODUCT PROD
 WHERE CUR.CURRENCY_ID = d.CURRENCY_ID
   AND PUR.PURCHASE_ID = d.PURCHASE_ID
   AND PROD.PRODUCT_ID = PUR.PRODUCT_ID
   AND PROD.bVALID     = 1               -- We want only valid entries
FOR READ ONLY
 
declare @d_SALES_ID     int
       ,@d_PURCHASE_ID  int
       ,@d_SHOP_ID      int
       ,@d_PRICEnc      numeric(10,2)
       ,@NAMEvc        varchar(64)
       ,@d_UNITSi       int
 
  OPEN crs_Hist
  WHILE (1 = 1)
  BEGIN
    FETCH crs_Hist INTO @d_SALES_ID ,@d_PURCHASE_ID ,@d_SHOP_ID ,@d_PRICEnc ,@NAMEvc ,@d_UNITSi
 
    IF (@@sqlstatus <> 0) BREAK -- Break loop once we finished this

    exec proc_SalesHistory 1, @d_SALES_ID, @d_PURCHASE_ID, @d_SHOP_ID, @d_PRICEnc, @NAMEvc, @d_UNITSi

    exec proc_ShopPerformance 1, 2, @d_PURCHASE_ID, @d_PRICEnc,@NAMEvc, "",@d_SHOP_ID
  END
  CLOSE crs_Hist
  DEALLOCATE CURSOR crs_Hist

end -- End of Trigger
