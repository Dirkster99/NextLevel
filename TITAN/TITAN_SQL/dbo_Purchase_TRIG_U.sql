
--
-- The Purchase update trigger is fired when records in the PURCHASE table are updated
-- Each change in a purchase changes the Purchase-History of a shop, the procedure
-- proc_PurchaseHistory does the processing
--
create trigger PURCHASE_TRIG_U on PURCHASE for UPDATE as
begin
  DECLARE crs_PurchHist CURSOR FOR -- Get all fields from that table
    SELECT
         i.PURCHASE_ID
        ,i.PURCHASE_PRICEnc
        ,CUR.NAMEvc
        ,i.UNITSi
    FROM inserted i, deleted d, CURRENCY CUR
   WHERE CUR.CURRENCY_ID = i.CURRENCY_ID
     AND d.PURCHASE_ID = i.PURCHASE_ID
     AND(
             d.VENDOR_ID        <> i.VENDOR_ID
          OR d.PRODUCT_ID       <> i.PRODUCT_ID
          OR d.PURCHASE_PRICEnc <> i.PURCHASE_PRICEnc
          OR d.CURRENCY_ID      <> i.CURRENCY_ID
          OR d.UNITSi           <> i.UNITSi
        )
  FOR READ ONLY
   
  declare
   @i_PURCHASE_ID int, @i_PURCHASE_PRICEnc numeric(10,2), @CUR_NAMEvc varchar(64), @i_UNITSi int

    OPEN crs_PurchHist
    WHILE (1 = 1)
    BEGIN
      FETCH crs_PurchHist INTO @i_PURCHASE_ID, @i_PURCHASE_PRICEnc, @CUR_NAMEvc, @i_UNITSi
   
      IF (@@sqlstatus <> 0) BREAK -- Hier gehts raus wenn alles abgearbeitet ist

      exec proc_PurchaseHistory 3, @i_PURCHASE_ID, @i_PURCHASE_PRICEnc, @CUR_NAMEvc, @i_UNITSi
    END
    CLOSE crs_PurchHist
    DEALLOCATE CURSOR crs_PurchHist

end
go
