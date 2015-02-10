--
-- The Purchase delete trigger is fired when records in the COSTS table are deleted
-- Each deleted record changes the Purchase-History of a shop, the procedure
-- proc_PurchaseHistory does the processing
--
create trigger PURCHASE_TRIG_D on PURCHASE for DELETE as
begin

  DECLARE crs_PurchHist CURSOR FOR -- Get all fields from that table
    SELECT d.PURCHASE_ID
          ,d.PURCHASE_PRICEnc
          ,CUR.NAMEvc
          ,d.UNITSi
      FROM deleted d, CURRENCY CUR
     WHERE CUR.CURRENCY_ID = d.CURRENCY_ID
  FOR READ ONLY
   
  declare
    @d_PURCHASE_ID      int
   ,@d_PURCHASE_PRICEnc numeric(10,2)
   ,@CUR_NAMEvc         varchar(64)
   ,@d_UNITSi           int
 
  OPEN crs_PurchHist
  WHILE (1 = 1)
  BEGIN
    FETCH crs_PurchHist INTO @d_PURCHASE_ID, @d_PURCHASE_PRICEnc, @CUR_NAMEvc, @d_UNITSi
 
    IF (@@sqlstatus <> 0) BREAK
 
    exec proc_PurchaseHistory 1, @d_PURCHASE_ID, @d_PURCHASE_PRICEnc, @CUR_NAMEvc, @d_UNITSi
  END
  CLOSE crs_PurchHist
  DEALLOCATE CURSOR crs_PurchHist

end
go
