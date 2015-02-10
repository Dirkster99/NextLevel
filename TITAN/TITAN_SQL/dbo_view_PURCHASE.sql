
--
-- This is just a simple view showing how nextlevel will resolve dependencies with views
-- Naturally, views can be access from other database objects via select, update, insert etc...
-- But note also, that views also depend themselfs on other database objects, namely, tables
-- or other views (via select access). Nextlevel deals with this via 2 seperate lists of dependencies.
--
create view view_PURCHASE as
select PUR.PURCHASE_ID, PUR.VENDOR_ID, PUR.PRODUCT_ID, PUR.PURCHASE_PRICEnc, PUR.CURRENCY_ID, PUR.UNITSi
      ,VDR.NAMEvc VDR_NAMEvc, VDR.COUNTRYvc
      ,PRD.NAMEvc PRD_NAMEvc, PRD.DESCRIPTIONvc, PRD.STDRD_PRICE_ID, PRD.SALES_PRICE_ID
      ,CAT.NAMEvc CAT_NAMEvc ,CAT.SUB_NAMEvc CAT_SUB_NAMEvc ,CAT.DESCRIPTIONvc CAT_DESCRIPTIONvc
  from PURCHASE PUR, VENDOR VDR, CATEGORY CAT, PRODUCT PRD
 where PUR.VENDOR_ID     = VDR.VENDOR_ID
   and PUR.PRODUCT_ID    = PRD.PRODUCT_ID
   and PRD.CATEGORY_1_ID = CAT.CAT_1_ID
   and PRD.CATEGORY_2_ID = CAT.CAT_2_ID
