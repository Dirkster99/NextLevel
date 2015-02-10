
--
-- The Purchase insert trigger is fired when records in the COSTS table are inserted
-- Each new purchase changes the Purchase-History of a shop, the procedure
-- proc_PurchaseHistory does the processing
--
create trigger PURCHASE_TRIG_I on PURCHASE for INSERT as
begin
  DECLARE crs_PurchHist CURSOR FOR -- Get all fields from that table
    SELECT i.PURCHASE_ID
          ,i.PURCHASE_PRICEnc
          ,CUR.NAMEvc
          ,i.UNITSi
      FROM inserted i, CURRENCY CUR
     WHERE CUR.CURRENCY_ID = i.CURRENCY_ID
  FOR READ ONLY
   
  declare
    @i_PURCHASE_ID      int
   ,@i_PURCHASE_PRICEnc numeric(10,2)
   ,@CUR_NAMEvc         varchar(64)
   ,@i_UNITSi           int
 
  OPEN crs_PurchHist
  WHILE (1 = 1)
  BEGIN
    FETCH crs_PurchHist INTO @i_PURCHASE_ID, @i_PURCHASE_PRICEnc, @CUR_NAMEvc, @i_UNITSi
 
    IF (@@sqlstatus <> 0) BREAK
 
    exec proc_PurchaseHistory 2, 2, @i_PURCHASE_ID, @i_PURCHASE_PRICEnc, @CUR_NAMEvc, @i_UNITSi
  END
  CLOSE crs_PurchHist
  DEALLOCATE CURSOR crs_PurchHist

end
go

