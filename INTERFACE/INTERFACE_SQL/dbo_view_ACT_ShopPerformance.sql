
--
-- This is just another view that shows how unreferenced objects will be recognized in NextLevel
-- The view itself depends on a table called Shop_Performance
-- It is however unreferenced, because no other database object makes use of it
-- (and the same could be true for a table or procedure, if they are defined but not used from within the DB)
--
create view view_ACT_ShopPerformance as
select SP_ID
      ,SHOP_ID
      ,SHOP_NAME
      ,SHOP_TYPE
  
      ,SHOP_ADDRESS_ID
      ,SHOP_COUNTRYvc
      ,SHOP_CITYvc
      ,SHOP_ZIPCODEvc
      ,SHOP_STREETvc
      ,SHOP_STATEvc
      ,SHOP_EMAILvc
      ,SHOP_WEB_SITEvc
      
      ,COST_ID
      ,PURCHASE_ID
      ,PRICE_TYPE
      ,PRICE
      ,PRICE_DESCRIPTION
      
      ,VERSION
      ,V_FROM
 FROM Shop_Performance
WHERE V_TO IS NULL

