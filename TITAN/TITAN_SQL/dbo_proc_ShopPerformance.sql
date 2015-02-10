
--
-- A shops performance is measured by computing the
-- sales values in a timmeframe against the costs to mantain the shop
--
create procedure proc_ShopPerformance
(
  @ACTION           int  -- 1=delete, 2=insert, 3=update
 ,@PRICE_TYPE       int
 ,@COST_PURCHASE_ID int  -- COST_ID or Purchase_ID when called from TRIGGER COST or PURCHASE

 ,@PRICE                 numeric(10,2) -- price in EURO can be cost or sales
 ,@NAMEvc                varchar(64)
 ,@PRICE_DESCRIPTION     varchar(255)

 ,@SHOP_ID               numeric(10,0)
)
AS
BEGIN

declare
  @SHOP_ADDRESS_ID int
 ,@SHOP_COUNTRYvc  varchar(64)
 ,@SHOP_CITYvc     varchar(64)
 ,@SHOP_ZIPCODEvc  varchar(64)
 ,@SHOP_STREETvc   varchar(64)
 ,@SHOP_STATEvc    varchar(64)
 ,@SHOP_EMAILvc    varchar(255)
 ,@SHOP_WEB_SITEvc varchar(255)
 ,@SHOP_NAME       varchar(64)
 ,@SHOP_TYPE       varchar(64)

 ,@DATESTAMP       datetime
 ,@VERSION         int
 ,@PRICEnc         numeric(10,2)
 ,@fix             bit
 ,@CntShop         int

  select @DATESTAMP = getdate(), @VERSION  = 1

  if @ACTION <> 1 -- No need for this to process a delete
  begin
    -- convert prices to EURO
    exec sp_exchange @NAMEvc, "EUR", @PRICEnc, @PRICEnc output, @fix output
  
    -- Get Sop information
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
  end

  if(@PRICE_TYPE = 1) -- Entry costs for this shop
  begin
    if exists(
      SELECT 1
        FROM Shop_Performance
       WHERE SHOP_ID = @SHOP_ID
         AND COST_ID = @COST_PURCHASE_ID
    )begin
      UPDATE Shop_Performance           -- Close last version and insert new record
         SET V_TO    = @DATESTAMP
       WHERE SHOP_ID = @SHOP_ID
         AND COST_ID = @COST_PURCHASE_ID
         AND V_TO IS NULL

      select @VERSION = max(VERSION) + 1 -- Get Version for new last record
        FROM Shop_Performance
       WHERE SHOP_ID = @SHOP_ID
         AND COST_ID = @COST_PURCHASE_ID
    end

  -- This is just another statemant showing how access to an external DB is implemented
   select @CntShop = count(*)
     from HERCULES..SHOPS

    if @ACTION <> 1                      -- We don't need this to process a delete
    begin                                -- The update above should have closed the last version
      INSERT Shop_Performance VALUES
      (
        @SHOP_ID
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
       ,@COST_PURCHASE_ID
       ,0
       ,@PRICE_TYPE
       ,@PRICEnc
       ,@PRICE_DESCRIPTION
       ,@VERSION
       ,@DATESTAMP
       ,NULL
     )
    end
  end      -- End of cost entry
  else
  if(@PRICE_TYPE = 2) -- Entry sale for this shop
  begin
    if exists(
      SELECT 1
        FROM Shop_Performance
       WHERE SHOP_ID     = @SHOP_ID
         AND PURCHASE_ID = @COST_PURCHASE_ID
    )begin
      UPDATE Shop_Performance           -- Close last version and insert new record
         SET V_TO        = @DATESTAMP
       WHERE SHOP_ID     = @SHOP_ID
         AND PURCHASE_ID = @COST_PURCHASE_ID
         AND V_TO IS NULL

      select @VERSION = max(VERSION) + 1 -- Get Version for new last record
        FROM Shop_Performance
       WHERE SHOP_ID     = @SHOP_ID
         AND PURCHASE_ID = @COST_PURCHASE_ID
    end

    if @ACTION <> 1                      -- We don't need this to process a delete
    begin                                -- The update above should have closed the last version
      INSERT Shop_Performance VALUES
      (
        @SHOP_ID
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
       ,0
       ,@COST_PURCHASE_ID
       ,@PRICE_TYPE
       ,@PRICEnc
       ,@PRICE_DESCRIPTION
       ,@VERSION
       ,@DATESTAMP
       ,NULL
     )
    end
  end    -- End of sales entry
END
