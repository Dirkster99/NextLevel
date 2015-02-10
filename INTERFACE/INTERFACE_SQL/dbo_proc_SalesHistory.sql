
--
-- Each change in a record is processed by the proc_SalesHistory table which in
-- turn is used to analyze changing Sales over time ...
-- The history is used to answer questions like:
--
-- How are shops sales affected by seasonal changes?
-- How are certain products priced at certain times?
--
create procedure proc_SalesHistory
(
 @ACTION        int           -- 1=delete, 2=insert, 3=update
,@SALES_ID    int
,@PURCHASE_ID int
,@SHOP_ID     int
,@PRICEnc     numeric(10,2)
,@NAMEvc        varchar(64)
,@UNITSi      int
)
AS
BEGIN
  -- Sanity Check
  if @ACTION <> 1 AND @PRICEnc < 0
  begin
    declare @PARAMSvc VARCHAR(255)

    select @PARAMSvc = convert(varchar, @SALES_ID)
               + "," + convert(varchar, @PURCHASE_ID)
               + "," + convert(varchar, @SHOP_ID)
               + "," + convert(varchar, @PRICEnc)
               + "," + convert(varchar, @NAMEvc)
               + "," + convert(varchar, @UNITSi)

    exec proc_Error "proc_SalesHistory", @PARAMSvc, "ERROR: Negative PRICE in Sales!"
    return -1
  end

declare
  @VENDOR_ID       int
 ,@VENDOR_NAME     varchar(64)
 ,@VENDOR_COUNTRY  varchar(64)

 ,@SHOP_NAME    VARCHAR(32)
 ,@SHOP_TYPE    VARCHAR(255)

 ,@PRODUCT_ID          int
 ,@PRODUCT_NAME        varchar(64)
 ,@PRODUCT_DESCRIPTION varchar(64)

 ,@CATEGORY_NAME     varchar(64)
 ,@CATEGORY_SUB_NAME varchar(64)

 ,@SHOP_ADDRESS_ID int
 ,@SHOP_COUNTRYvc  varchar(64)
 ,@SHOP_CITYvc     varchar(64)
 ,@SHOP_ZIPCODEvc  varchar(64)
 ,@SHOP_STREETvc   varchar(64)
 ,@SHOP_STATEvc    varchar(64)
 ,@SHOP_EMAILvc    varchar(255)
 ,@SHOP_WEB_SITEvc varchar(255)

 ,@DATESTAMP  datetime
 ,@VERSION          int
 ,@fix              bit
 ,@bValid           bit

  select @DATESTAMP = getdate(), @VERSION  = 1,@bValid = 0

  --Here is one mor eof this on another imaginary server called SUN
  exec @retCODEi = SUN . TITAN .dbo.proc_PurchaseHistory @ACTION, @PURCHASE_ID ,@PURCHASE_PRICEnc ,@CUR_NAMEvc ,@UNITSi

  if @ACTION <> 1 -- No need for this to process a delete
  begin
    -- Get Shop information
    SELECT @SHOP_NAME       = SH.NAMEvc
          ,@SHOP_TYPE       = SH.TYPE
          ,@SHOP_ADDRESS_ID = ADR.ADDRESS_ID
          ,@SHOP_COUNTRYvc  = ADR.COUNTRYvc
          ,@SHOP_CITYvc     = ADR.CITYvc
          ,@SHOP_ZIPCODEvc  = ADR.ZIPCODEvc
          ,@SHOP_STREETvc   = ADR.STREETvc
          ,@SHOP_STATEvc    = ADR.STATEvc
          ,@SHOP_EMAILvc    = ADR.EMAILvc
          ,@SHOP_WEB_SITEvc = ADR.WEB_SITEvc
      FROM SHOP SH, ADDRESS ADR
     WHERE SH.SHOP_ID    = @SHOP_ID
       AND SH.ADDRESS_ID = ADR.ADDRESS_ID

    -- Get Vendor information
    SELECT @VENDOR_ID        = VDR.VENDOR_ID
          ,@VENDOR_NAME        = VDR.NAMEvc
          ,@VENDOR_COUNTRY      = VDR.COUNTRYvc
          ,@PRODUCT_ID           = PROD.PRODUCT_ID
          ,@PRODUCT_NAME         = PROD.NAMEvc
          ,@PRODUCT_DESCRIPTION = PROD.DESCRIPTIONvc
          ,@CATEGORY_NAME      = CAT.NAMEvc
          ,@CATEGORY_SUB_NAME = CAT.SUB_NAMEvc
          ,@bValid           = PROD.bVALID
      FROM VENDOR VDR, PURCHASE PRC, PRODUCT PROD, CATEGORY CAT
     WHERE PRC.PURCHASE_ID    = @PURCHASE_ID
       AND VDR.VENDOR_ID      = PRC.VENDOR_ID
       AND PRC.PRODUCT_ID     = PROD.PRODUCT_ID
       AND PROD.CATEGORY_1_ID = CAT.CAT_1_ID
       AND PROD.CATEGORY_2_ID = CAT.CAT_2_ID
       AND PROD.bVALID        = 1               -- We want only valid entries

    -- convert prices to EURO
    exec sp_exchange @NAMEvc, "EUR", @PRICEnc, @PRICEnc output, @fix output
  end

  if exists(
  select 1
    from SALES_HISTORY
   where SHOP_ID    = @SHOP_ID
     and PRODUCT_ID = @PRODUCT_ID
  )begin
    UPDATE SALES_HISTORY              -- Close last version and insert new record
       SET V_TO        = @DATESTAMP
     WHERE SHOP_ID     = @SHOP_ID
       and PRODUCT_ID  = @PRODUCT_ID
       and VENDOR_ID   = @VENDOR_ID
       AND V_TO IS NULL

    select @VERSION = max(VERSION) + 1 -- Get Version for new last record
    from SALES_HISTORY
   where SHOP_ID    = @SHOP_ID
     and PRODUCT_ID = @PRODUCT_ID
     and VENDOR_ID  = @VENDOR_ID
  end

  if @ACTION <> 1                      -- We don't need this to process a delete
  begin                                -- The update above should have closed the last version
    if @bValid is not null             -- This will close the history when a product becomes unavailable
    begin
      INSERT SALES_HISTORY VALUES -- INSERT FIRST TIME/VERSIONED RECORD
      (  @SALES_ID
        ,@PRICEnc
        ,@UNITSi
        ,@VENDOR_ID
        ,@VENDOR_NAME
        ,@VENDOR_COUNTRY
        ,@PRODUCT_ID
        ,@PRODUCT_NAME
        ,@PRODUCT_DESCRIPTION
        ,@CATEGORY_NAME
        ,@CATEGORY_SUB_NAME
        ,@SHOP_ID
        ,@SHOP_NAME
        ,@SHOP_TYPE
        ,@SHOP_ADDRESS_ID
        ,@SHOP_COUNTRYvc
        ,@SHOP_CITYvc
        ,@SHOP_ZIPCODEvc
        ,@SHOP_STREETvc
        ,@SHOP_STATEvc
        ,@SHOP_EMAILvc
        ,@SHOP_WEB_SITEvc
        ,@VERSION
        ,@DATESTAMP
        ,null
      )
    end
  end

  return 0
END

