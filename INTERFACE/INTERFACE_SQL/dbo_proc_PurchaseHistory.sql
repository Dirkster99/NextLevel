
--
-- PurchaseHistory gives an overview on the purchases
-- for products that are later sold, this is useful to answer questions such as
-- Who is the best vendor in a certain timeframe? etc...
--
create procedure proc_PurchaseHistory
(
  @ACTION      int         -- 1=DELETE, 2=INSERT, 3=UPDATE
 ,@PURCHASE_ID int

 ,@PURCHASE_PRICEnc numeric(10,2)
 ,@CUR_NAMEvc       varchar(64)
 ,@UNITSi           int
)
AS
BEGIN

declare
  @DATESTAMP  datetime
 ,@VERSION    int
 ,@PRICEnc    numeric(10,2)
 ,@fix        bit

 ,@VENDOR_ID         int
 ,@VDR_NAMEvc        varchar(64)
 ,@COUNTRYvc         varchar(64)
 ,@PRODUCT_ID        int
 ,@PRD_NAMEvc        varchar(64)
 ,@DESCRIPTIONvc     varchar(64)
 ,@CAT_NAMEvc        varchar(64)
 ,@CAT_SUB_NAMEvc    varchar(64)
 ,@CAT_DESCRIPTIONvc varchar(64)
 ,@retCODEi          int

  select @DATESTAMP = getdate(), @VERSION  = 1

  --This is just a sample statement for an execution of the same procedure on an imaginary server called MOON
  exec @retCODEi = MOON . TITAN .dbo.proc_PurchaseHistory @ACTION, @PURCHASE_ID ,@PURCHASE_PRICEnc ,@CUR_NAMEvc ,@UNITSi

  if @ACTION <> 1  -- We don't need this to process a delete
  begin
    -- convert prices to EURO
    exec sp_exchange @CUR_NAMEvc, "EUR", @PURCHASE_PRICEnc, @PRICEnc output, @fix output
  
    select @VENDOR_ID      = VENDOR_ID
          ,@VDR_NAMEvc     = VDR_NAMEvc
          ,@COUNTRYvc      = COUNTRYvc
          ,@PRODUCT_ID     = PRODUCT_ID
          ,@PRD_NAMEvc     = PRD_NAMEvc
          ,@DESCRIPTIONvc  = DESCRIPTIONvc
          ,@CAT_NAMEvc        = CAT_NAMEvc
          ,@CAT_SUB_NAMEvc    = CAT_SUB_NAMEvc
          ,@CAT_DESCRIPTIONvc = CAT_DESCRIPTIONvc
      from view_PURCHASE
     where PURCHASE_ID = @PURCHASE_ID
  end

  --Here is one more of this one on another imaginary server called SUN
  exec @retCODEi = SUN. TITAN .dbo.proc_PurchaseHistory1 @ACTION, @PURCHASE_ID ,@PURCHASE_PRICEnc ,@CUR_NAMEvc ,@UNITSi

  if exists
  (
    select 1
      from PURCHASE_HISTORY
     where PURCHASE_ID = @PURCHASE_ID
  )
  begin                            -- This is not the first version of this record
    UPDATE PURCHASE_HISTORY        -- Close last version and insert new record
       SET V_TO        = @DATESTAMP
     WHERE PURCHASE_ID = @PURCHASE_ID
       AND V_TO IS NULL

    select @VERSION = max(VERSION) + 1 -- Get Version for new last record
     from PURCHASE_HISTORY
    where PURCHASE_ID = @PURCHASE_ID
  end

  if @ACTION <> 1                      -- We don't need this to process a delete
  begin                                -- The update above should have closed the last version
    INSERT PURCHASE_HISTORY VALUES
    (
      @PURCHASE_ID
     ,@PRICEnc        -- price in EURO of sales
     ,@UNITSi
     ,@VENDOR_ID
     ,@VDR_NAMEvc
     ,@COUNTRYvc
     ,@PRODUCT_ID
     ,@PRD_NAMEvc
     ,@PRD_NAMEvc
     ,@CAT_NAMEvc
     ,@CAT_SUB_NAMEvc
     ,@VERSION              -- VERSION and datetime of validity
     ,@DATESTAMP
     ,NULL
    )
  end

  return 0
END
