--
-- Create a demo database called TITAN and all its object schema definitions
-- (tables, views, procedures, triggers)
--
-- Create Demo Database script based on script generator from SQL Server 2014
--
USE [master]
GO

/****** Object:  Database [TITAN]    Script Date: 06.05.2015 21:24:39 ******/
CREATE DATABASE [TITAN]
GO

USE [TITAN]
GO
/****** Object:  Table [dbo].[ADDRESS]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[ADDRESS](
	[ADDRESS_ID] [int] NOT NULL,
	[COUNTRYvc] [varchar](64) NOT NULL,
	[CITYvc] [varchar](64) NOT NULL,
	[ZIPCODEvc] [varchar](64) NOT NULL,
	[STREETvc] [varchar](64) NOT NULL,
	[STATEvc] [varchar](64) NOT NULL,
	[EMAILvc] [varchar](255) NOT NULL,
	[WEB_SITEvc] [varchar](255) NOT NULL
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[CATEGORY]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[CATEGORY](
	[CAT_1_ID] [int] NOT NULL,
	[CAT_2_ID] [int] NOT NULL,
	[NAMEvc] [varchar](64) NOT NULL,
	[SUB_NAMEvc] [varchar](64) NOT NULL,
	[DESCRIPTIONvc] [varchar](64) NOT NULL
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[COST_TYPES]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[COST_TYPES](
	[TYPE_ID] [int] NOT NULL,
	[DESCRIPTIONvc] [varchar](64) NOT NULL
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[COSTS]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[COSTS](
	[COST_ID] [int] NOT NULL,
	[SHOP_ID] [int] NOT NULL,
	[TYPE_ID] [int] NOT NULL,
	[PRICE] [numeric](10, 2) NOT NULL,
	[CURRENCY_ID] [int] NOT NULL,
	[DateStamp] [datetime] NOT NULL
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[CURRENCY]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[CURRENCY](
	[CURRENCY_ID] [int] NOT NULL,
	[NAMEvc] [varchar](64) NOT NULL
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[ERROR_MSG]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[ERROR_MSG](
	[ID] [numeric](10, 0) IDENTITY(1,1) NOT NULL,
	[NOWdt] [datetime] NULL,
	[SOURCEvc] [varchar](32) NULL,
	[PARAMSvc] [varchar](255) NULL,
	[MESSAGEvc] [varchar](255) NULL
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[PRODUCT]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[PRODUCT](
	[Product_ID] [int] NOT NULL,
	[NAMEvc] [varchar](64) NOT NULL,
	[DESCRIPTIONvc] [varchar](255) NOT NULL,
	[STDRD_PRICE_ID] [int] NOT NULL,
	[SALES_PRICE_ID] [int] NOT NULL,
	[CATEGORY_1_ID] [int] NOT NULL,
	[CATEGORY_2_ID] [int] NOT NULL,
	[bVALID] [bit] NOT NULL
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[PRODUCT_PRICE]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[PRODUCT_PRICE](
	[Product_ID] [int] NOT NULL,
	[NAMEvc] [varchar](64) NOT NULL,
	[DESCRIPTIONvc] [varchar](255) NOT NULL,
	[bVALID] [bit] NOT NULL,
	[STDRD_PRICE_ID] [int] NOT NULL,
	[STDRD_VALID_FROMdt] [datetime] NOT NULL,
	[STDRD_VALID_TOdt] [datetime] NOT NULL,
	[STDRD_PRICEnc] [numeric](10, 2) NOT NULL,
	[STDRD_CURRENCY_ID] [varchar](64) NOT NULL,
	[SALES_PRICE_ID] [int] NOT NULL,
	[SALES_VALID_FROMdt] [datetime] NOT NULL,
	[SALES_VALID_TOdt] [datetime] NOT NULL,
	[SALES_PRICEnc] [numeric](10, 2) NOT NULL,
	[SALES_CURRENCY_ID] [int] NOT NULL,
	[SALES_DESCRIPTIONvc] [varchar](255) NOT NULL,
	[SALES_NAMEvc] [varchar](64) NOT NULL,
	[CAT_NAMEvc] [varchar](64) NOT NULL,
	[CAT_SUB_NAMEvc] [varchar](64) NOT NULL,
	[CAT_DESCRIPTIONvc] [varchar](64) NOT NULL
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[PURCHASE]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PURCHASE](
	[PURCHASE_ID] [int] NOT NULL,
	[VENDOR_ID] [int] NOT NULL,
	[PRODUCT_ID] [int] NOT NULL,
	[PURCHASE_PRICEnc] [numeric](10, 2) NOT NULL,
	[CURRENCY_ID] [int] NOT NULL,
	[UNITSi] [int] NOT NULL
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[PURCHASE_HISTORY]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[PURCHASE_HISTORY](
	[PH_ID] [numeric](10, 0) IDENTITY(1,1) NOT NULL,
	[PURCHASE_ID] [int] NOT NULL,
	[PURCHASE_PRICE] [numeric](10, 2) NOT NULL,
	[PURCHASE_UNITS] [int] NOT NULL,
	[VENDOR_ID] [int] NOT NULL,
	[VENDOR_NAME] [varchar](64) NOT NULL,
	[VENDOR_COUNTRY] [varchar](64) NOT NULL,
	[PRODUCT_ID] [int] NOT NULL,
	[PRODUCT_NAME] [varchar](64) NOT NULL,
	[PRODUCT_DESCRIPTION] [varchar](64) NOT NULL,
	[CATEGORY_NAME] [varchar](64) NOT NULL,
	[CATEGORY_SUB_NAME] [varchar](64) NOT NULL,
	[VERSION] [int] NOT NULL,
	[V_FROM] [datetime] NOT NULL,
	[V_TO] [datetime] NULL
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[SALES]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[SALES](
	[SALES_ID] [int] NOT NULL,
	[PURCHASE_ID] [int] NOT NULL,
	[SHOP_ID] [int] NOT NULL,
	[PRICEnc] [numeric](10, 2) NOT NULL,
	[CURRENCY_ID] [int] NOT NULL,
	[UNITSi] [int] NOT NULL
) ON [PRIMARY]

GO
/****** Object:  Table [dbo].[SALES_HISTORY]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[SALES_HISTORY](
	[SH_ID] [numeric](10, 0) IDENTITY(1,1) NOT NULL,
	[SALES_ID] [int] NOT NULL,
	[SALES_PRICE] [numeric](10, 2) NOT NULL,
	[SALES_UNITS] [int] NOT NULL,
	[VENDOR_ID] [int] NOT NULL,
	[VENDOR_NAME] [varchar](64) NOT NULL,
	[VENDOR_COUNTRY] [varchar](64) NOT NULL,
	[PRODUCT_ID] [int] NOT NULL,
	[PRODUCT_NAME] [varchar](64) NOT NULL,
	[PRODUCT_DESCRIPTION] [varchar](64) NOT NULL,
	[CATEGORY_NAME] [varchar](64) NOT NULL,
	[CATEGORY_SUB_NAME] [varchar](64) NOT NULL,
	[SHOP_ID] [int] NOT NULL,
	[SHOP_NAME] [varchar](32) NOT NULL,
	[SHOP_TYPE] [varchar](255) NOT NULL,
	[SHOP_ADDRESS_ID] [int] NOT NULL,
	[SHOP_COUNTRYvc] [varchar](64) NOT NULL,
	[SHOP_CITYvc] [varchar](64) NOT NULL,
	[SHOP_ZIPCODEvc] [varchar](64) NOT NULL,
	[SHOP_STREETvc] [varchar](64) NOT NULL,
	[SHOP_STATEvc] [varchar](64) NOT NULL,
	[SHOP_EMAILvc] [varchar](255) NOT NULL,
	[SHOP_WEB_SITEvc] [varchar](255) NOT NULL,
	[VERSION] [int] NOT NULL,
	[V_FROM] [datetime] NOT NULL,
	[V_TO] [datetime] NULL
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[SALESPRICE]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[SALESPRICE](
	[PRICE_ID] [int] NOT NULL,
	[VALID_FROMdt] [datetime] NOT NULL,
	[VALID_TOdt] [datetime] NOT NULL,
	[PRICEnc] [numeric](10, 2) NOT NULL,
	[CURRENCY_ID] [int] NOT NULL,
	[DESCRIPTIONvc] [varchar](255) NOT NULL,
	[NAMEvc] [varchar](64) NOT NULL
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[SHOP]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[SHOP](
	[SHOP_ID] [int] NOT NULL,
	[ADDRESS_ID] [int] NOT NULL,
	[NAMEvc] [varchar](64) NOT NULL,
	[TYPE] [varchar](64) NOT NULL
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[Shop_Performance]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[Shop_Performance](
	[SP_ID] [numeric](10, 0) IDENTITY(1,1) NOT NULL,
	[SHOP_ID] [int] NOT NULL,
	[SHOP_NAME] [varchar](32) NOT NULL,
	[SHOP_TYPE] [varchar](255) NOT NULL,
	[SHOP_ADDRESS_ID] [int] NOT NULL,
	[SHOP_COUNTRYvc] [varchar](64) NOT NULL,
	[SHOP_CITYvc] [varchar](64) NOT NULL,
	[SHOP_ZIPCODEvc] [varchar](64) NOT NULL,
	[SHOP_STREETvc] [varchar](64) NOT NULL,
	[SHOP_STATEvc] [varchar](64) NOT NULL,
	[SHOP_EMAILvc] [varchar](255) NOT NULL,
	[SHOP_WEB_SITEvc] [varchar](255) NOT NULL,
	[COST_ID] [int] NULL,
	[PURCHASE_ID] [int] NULL,
	[PRICE_TYPE] [int] NOT NULL,
	[PRICE] [numeric](10, 2) NOT NULL,
	[PRICE_DESCRIPTION] [varchar](255) NOT NULL,
	[VERSION] [int] NOT NULL,
	[V_FROM] [datetime] NOT NULL,
	[V_TO] [datetime] NULL
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[STANDARDPRICE]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[STANDARDPRICE](
	[PRICE_ID] [int] NOT NULL,
	[VALID_FROMdt] [datetime] NOT NULL,
	[VALID_TOdt] [datetime] NOT NULL,
	[PRICEnc] [numeric](10, 2) NOT NULL,
	[CURRENCY_ID] [varchar](64) NOT NULL
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [dbo].[VENDOR]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[VENDOR](
	[VENDOR_ID] [int] NOT NULL,
	[NAMEvc] [varchar](64) NOT NULL,
	[COUNTRYvc] [varchar](64) NOT NULL
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  View [dbo].[view_ACT_ShopPerformance]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
--
-- This is just another view that shows how unreferenced objects will be recognized in NextLevel
-- The view itself depends on a table called Shop_Performance
-- It is however unreferenced, because no other database object makes use of it
-- (and the same could be true for a table or procedure, if they are defined but not used from within the DB)
--
create view [dbo].[view_ACT_ShopPerformance] as
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

GO
/****** Object:  View [dbo].[view_PURCHASE]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO

--
-- This is just a simple view showing how nextlevel will resolve dependencies with views
-- Naturally, views can be access from other database objects via select, update, insert etc...
-- But note also, that views also depend themselfs on other database objects, namely, tables
-- or other views (via select access). Nextlevel deals with this via 2 seperate lists of dependencies.
--
create view [dbo].[view_PURCHASE] as
select PUR.PURCHASE_ID, PUR.VENDOR_ID, PUR.PRODUCT_ID, PUR.PURCHASE_PRICEnc, PUR.CURRENCY_ID, PUR.UNITSi
      ,VDR.NAMEvc VDR_NAMEvc, VDR.COUNTRYvc
      ,PRD.NAMEvc PRD_NAMEvc, PRD.DESCRIPTIONvc, PRD.STDRD_PRICE_ID, PRD.SALES_PRICE_ID
      ,CAT.NAMEvc CAT_NAMEvc ,CAT.SUB_NAMEvc CAT_SUB_NAMEvc ,CAT.DESCRIPTIONvc CAT_DESCRIPTIONvc
  from PURCHASE PUR, VENDOR VDR, CATEGORY CAT, PRODUCT PRD
 where PUR.VENDOR_ID     = VDR.VENDOR_ID
   and PUR.PRODUCT_ID    = PRD.PRODUCT_ID
   and PRD.CATEGORY_1_ID = CAT.CAT_1_ID
   and PRD.CATEGORY_2_ID = CAT.CAT_2_ID

GO
/****** Object:  Index [ADDRESS_ID_ind]    Script Date: 06.05.2015 21:24:39 ******/
CREATE UNIQUE NONCLUSTERED INDEX [ADDRESS_ID_ind] ON [dbo].[ADDRESS]
(
	[ADDRESS_ID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [CATEGORY_ind]    Script Date: 06.05.2015 21:24:39 ******/
CREATE UNIQUE NONCLUSTERED INDEX [CATEGORY_ind] ON [dbo].[CATEGORY]
(
	[CAT_1_ID] ASC,
	[CAT_2_ID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [COST_TYPES_ID_ind]    Script Date: 06.05.2015 21:24:39 ******/
CREATE UNIQUE NONCLUSTERED INDEX [COST_TYPES_ID_ind] ON [dbo].[COST_TYPES]
(
	[TYPE_ID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [COSTS_ID_ind]    Script Date: 06.05.2015 21:24:39 ******/
CREATE UNIQUE NONCLUSTERED INDEX [COSTS_ID_ind] ON [dbo].[COSTS]
(
	[COST_ID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [currency_id_ind]    Script Date: 06.05.2015 21:24:39 ******/
CREATE UNIQUE NONCLUSTERED INDEX [currency_id_ind] ON [dbo].[CURRENCY]
(
	[CURRENCY_ID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [ERROR_MSG_ID_ind]    Script Date: 06.05.2015 21:24:39 ******/
CREATE UNIQUE NONCLUSTERED INDEX [ERROR_MSG_ID_ind] ON [dbo].[ERROR_MSG]
(
	[ID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [product_id_ind]    Script Date: 06.05.2015 21:24:39 ******/
CREATE UNIQUE NONCLUSTERED INDEX [product_id_ind] ON [dbo].[PRODUCT]
(
	[Product_ID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [PRODUCT_PRICE_id_ind]    Script Date: 06.05.2015 21:24:39 ******/
CREATE UNIQUE NONCLUSTERED INDEX [PRODUCT_PRICE_id_ind] ON [dbo].[PRODUCT_PRICE]
(
	[Product_ID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [purchase_id_ind]    Script Date: 06.05.2015 21:24:39 ******/
CREATE UNIQUE NONCLUSTERED INDEX [purchase_id_ind] ON [dbo].[PURCHASE]
(
	[PURCHASE_ID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [PURCHASE_HISTORY_ID_ind]    Script Date: 06.05.2015 21:24:39 ******/
CREATE UNIQUE NONCLUSTERED INDEX [PURCHASE_HISTORY_ID_ind] ON [dbo].[PURCHASE_HISTORY]
(
	[PH_ID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [PURCHASE_HISTORY_VERS_ind]    Script Date: 06.05.2015 21:24:39 ******/
CREATE UNIQUE NONCLUSTERED INDEX [PURCHASE_HISTORY_VERS_ind] ON [dbo].[PURCHASE_HISTORY]
(
	[VERSION] ASC,
	[V_FROM] ASC,
	[V_TO] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [SALES_ID_ind]    Script Date: 06.05.2015 21:24:39 ******/
CREATE UNIQUE NONCLUSTERED INDEX [SALES_ID_ind] ON [dbo].[SALES]
(
	[SALES_ID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [SALES_HISTORY_ID_ind]    Script Date: 06.05.2015 21:24:39 ******/
CREATE UNIQUE NONCLUSTERED INDEX [SALES_HISTORY_ID_ind] ON [dbo].[SALES_HISTORY]
(
	[SH_ID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [SALES_HISTORY_VERS_ind]    Script Date: 06.05.2015 21:24:39 ******/
CREATE UNIQUE NONCLUSTERED INDEX [SALES_HISTORY_VERS_ind] ON [dbo].[SALES_HISTORY]
(
	[VERSION] ASC,
	[V_FROM] ASC,
	[V_TO] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [salesprice_id_ind]    Script Date: 06.05.2015 21:24:39 ******/
CREATE UNIQUE NONCLUSTERED INDEX [salesprice_id_ind] ON [dbo].[SALESPRICE]
(
	[PRICE_ID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [shop_id_ind]    Script Date: 06.05.2015 21:24:39 ******/
CREATE UNIQUE NONCLUSTERED INDEX [shop_id_ind] ON [dbo].[SHOP]
(
	[SHOP_ID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [Shop_Performance_ID_ind]    Script Date: 06.05.2015 21:24:39 ******/
CREATE UNIQUE NONCLUSTERED INDEX [Shop_Performance_ID_ind] ON [dbo].[Shop_Performance]
(
	[SP_ID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [Shop_Performance_VERS_ind]    Script Date: 06.05.2015 21:24:39 ******/
CREATE UNIQUE NONCLUSTERED INDEX [Shop_Performance_VERS_ind] ON [dbo].[Shop_Performance]
(
	[VERSION] ASC,
	[V_FROM] ASC,
	[V_TO] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [standard_id_ind]    Script Date: 06.05.2015 21:24:39 ******/
CREATE UNIQUE NONCLUSTERED INDEX [standard_id_ind] ON [dbo].[STANDARDPRICE]
(
	[PRICE_ID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [vendor_id_ind]    Script Date: 06.05.2015 21:24:39 ******/
CREATE UNIQUE NONCLUSTERED INDEX [vendor_id_ind] ON [dbo].[VENDOR]
(
	[VENDOR_ID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  StoredProcedure [dbo].[proc_Error]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO

-- proc_Error is called whenever errors need to be locked away
-- in order to have a history of unprocessed events/data or other problems
CREATE procedure [dbo].[proc_Error]
(
  @SOURCEvc  VARCHAR(32)
 ,@PARAMSvc  VARCHAR(255)
 ,@MESSAGEvc VARCHAR(255)
)
AS
BEGIN
  INSERT INTO ERROR_MSG(NOWdt, SOURCEvc, PARAMSvc, MESSAGEvc)
  VALUES(getdate(), @SOURCEvc ,@PARAMSvc ,@MESSAGEvc)

END


GO
/****** Object:  StoredProcedure [dbo].[proc_PurchaseHistory]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
--
-- PurchaseHistory gives an overview on the purchases
-- for products that are later sold, this is useful to answer questions such as
-- Who is the best vendor in a certain timeframe? etc...
--
CREATE procedure [dbo].[proc_PurchaseHistory]
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
  exec @retCODEi = SUN. INTERFACE .dbo.proc_PurchaseHistory @ACTION, @PURCHASE_ID ,@PURCHASE_PRICEnc ,@CUR_NAMEvc ,@UNITSi

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


GO
/****** Object:  StoredProcedure [dbo].[proc_SalesHistory]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE procedure [dbo].[proc_SalesHistory]
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
               + ',' + convert(varchar, @PURCHASE_ID)
               + ',' + convert(varchar, @SHOP_ID)
               + ',' + convert(varchar, @PRICEnc)
               + ',' + convert(varchar, @NAMEvc)
               + ',' + convert(varchar, @UNITSi)

    exec proc_Error "proc_SalesHistory", @PARAMSvc, "ERROR: Negative PRICE in Sales!"
    return -1
  end

declare
  @VENDOR_ID       int
 ,@retCODEi        int
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
 ,@PURCHASE_PRICEnc numeric(8,2)
 ,@CUR_NAMEvc       varchar(64)

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


GO
/****** Object:  StoredProcedure [dbo].[proc_ShopPerformance]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE procedure [dbo].[proc_ShopPerformance]
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


GO
/****** Object:  Trigger [dbo].[COSTTYPE_TRIG_D]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
--
-- The COSTTYPE delete trigger is fired when records in the COSTTYPE table are deleted
-- Costtypes in existence are not expected to change, if they do, manual wor on the
-- database is required to reflect the change, therefor we do error reporting here
-- and the developer/dba is expected to do the rest
--
create trigger [dbo].[COSTTYPE_TRIG_D] on [dbo].[COST_TYPES] for DELETE as
begin

  DECLARE crs_COSTTYPE CURSOR FOR -- Get all fields from that table
    SELECT d.TYPE_ID, d.DESCRIPTIONvc
      FROM deleted d
  FOR READ ONLY
 
  declare
   @Error_Text      varchar(1024)
  ,@d_TYPE_ID       int
  ,@d_DESCRIPTIONvc varchar(64)
 
  OPEN crs_COSTTYPE
  WHILE (1 = 1)
  BEGIN
    FETCH crs_COSTTYPE INTO @d_TYPE_ID, @d_DESCRIPTIONvc
 
    IF (@@FETCH_STATUS <> 0) BREAK -- Get out of here if there is no data left to process
 
    select @Error_Text =  ' TYPE_ID='       + convert(varchar, @d_TYPE_ID)
                        + ',DESCRIPTIONvc=' + convert(varchar, @d_DESCRIPTIONvc)
 
    exec proc_Error 'COST_TYPE_TRIG_D', @Error_Text, 'ERROR: Unexpected DELETE on COST_TYPE!!' 
  END
  CLOSE crs_COSTTYPE
  DEALLOCATE crs_COSTTYPE

end

GO
/****** Object:  Trigger [dbo].[COSTTYPE_TRIG_U]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO

--
-- The COSTTYPE update trigger is fired when records in the COSTTYPE table are updated
-- Costtypes in existence are not expected to change, if they do, manual work on the
-- database is required to reflect the change, therefor we do error reporting here
-- and the developer/dba is expected to do the rest
--
create trigger [dbo].[COSTTYPE_TRIG_U] on [dbo].[COST_TYPES] for UPDATE as
begin
  DECLARE crs_COSTTYPE CURSOR FOR -- Get all fields from that table
    SELECT d.TYPE_ID      , i.TYPE_ID                         
          ,d.DESCRIPTIONvc, i.DESCRIPTIONvc                   
      FROM inserted i, deleted d
     WHERE d.TYPE_ID        = i.TYPE_ID
       AND d.DESCRIPTIONvc <> i.DESCRIPTIONvc
  FOR READ ONLY
   
  declare
  @Error_Text varchar(1024)
 ,@d_TYPE_ID       int         , @i_TYPE_ID       int
 ,@d_DESCRIPTIONvc varchar(64) , @i_DESCRIPTIONvc varchar(64)
 
  OPEN crs_COSTTYPE
  WHILE (1 = 1)
  BEGIN
    FETCH crs_COSTTYPE INTO @d_TYPE_ID, @i_TYPE_ID
                           ,@d_DESCRIPTIONvc, @i_DESCRIPTIONvc
 
    IF (@@FETCH_STATUS <> 0) BREAK -- Get out of here if there is no data left to process
 
    select @Error_Text = ' TYPE_ID='       + convert(varchar, @d_TYPE_ID)
                       + ',DESCRIPTIONvc=' + @d_DESCRIPTIONvc + ' => ' + @i_DESCRIPTIONvc
 
    exec proc_Error 'COST_TYPE_TRIG_U', @Error_Text, 'ERROR: Unexpected UPDATE on COST_TYPE!!'
 
  END
  CLOSE crs_COSTTYPE
  DEALLOCATE crs_COSTTYPE

end

GO
/****** Object:  Trigger [dbo].[COSTS_TRIG_D]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO


--
-- The COSTS delete trigger is fired when records in the COSTS table are deleted
-- Each deletion of a record changes the performance of a shop, the procedure
-- proc_ShopPerformance does the processing
--
create trigger [dbo].[COSTS_TRIG_D] on [dbo].[COSTS] for DELETE as
begin

  DECLARE crs_ShopPerform CURSOR FOR -- Get all fields from that table
    SELECT d.COST_ID
          ,d.SHOP_ID
          ,d.PRICE
          ,CUR.NAMEvc
          ,CTP.DESCRIPTIONvc
     FROM deleted d, COST_TYPES CTP, CURRENCY CUR
    WHERE CUR.CURRENCY_ID = d.CURRENCY_ID
      AND d.TYPE_ID       = CTP.TYPE_ID
  FOR READ ONLY

  declare
   @Error_Text      varchar(1024)
  ,@d_COST_ID       int
  ,@d_SHOP_ID       int
  ,@d_DESCRIPTIONvc varchar(64)
  ,@d_PRICE         numeric(10,2)
  ,@d_NAMEvc        varchar(64)

    OPEN crs_ShopPerform
    WHILE (1 = 1)
    BEGIN
      FETCH crs_ShopPerform INTO @d_COST_ID,@d_SHOP_ID,@d_PRICE,@d_NAMEvc,@d_DESCRIPTIONvc
   
      IF (@@FETCH_STATUS <> 0) BREAK -- Break loop once we finished this
   
      exec proc_ShopPerformance 1, 1, @d_COST_ID, @d_PRICE,@d_NAMEvc, @d_DESCRIPTIONvc,@d_SHOP_ID

    END
    CLOSE crs_ShopPerform
    DEALLOCATE crs_ShopPerform

end

GO
/****** Object:  Trigger [dbo].[COSTS_TRIG_I]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO

--
-- The COSTS insert trigger is fired when records in the COSTS table are inserted
-- Each new record changes the performance of a shop, the procedure
-- proc_ShopPerformance does the processing
--
create trigger [dbo].[COSTS_TRIG_I] on [dbo].[COSTS] for INSERT as
begin
  DECLARE crs_ShopPerform CURSOR FOR -- Get all fields from that table
    SELECT i.COST_ID
          ,i.SHOP_ID
          ,i.PRICE
          ,CUR.NAMEvc
          ,CTP.DESCRIPTIONvc
     FROM inserted i, COST_TYPES CTP, CURRENCY CUR
    WHERE CUR.CURRENCY_ID = i.CURRENCY_ID
      AND i.TYPE_ID = CTP.TYPE_ID
  FOR READ ONLY

  declare
   @Error_Text varchar(1024)
  ,@i_COST_ID     int
  ,@i_SHOP_ID     int
  ,@i_DESCRIPTIONvc varchar(64)
  ,@i_PRICE       numeric(10,2)
  ,@i_NAMEvc        varchar(64)

    OPEN crs_ShopPerform
    WHILE (1 = 1)
    BEGIN
      FETCH crs_ShopPerform INTO @i_COST_ID, @i_SHOP_ID, @i_PRICE, @i_NAMEvc, @i_DESCRIPTIONvc
   
      IF (@@FETCH_STATUS <> 0) BREAK -- Break loop once we finished this
   
      exec proc_ShopPerformance 2, 1, @i_COST_ID, @i_PRICE,@i_NAMEvc, @i_DESCRIPTIONvc,@i_SHOP_ID
   
    END
    CLOSE crs_ShopPerform
    DEALLOCATE crs_ShopPerform

end

GO
/****** Object:  Trigger [dbo].[COSTS_TRIG_U]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO

--
-- The COSTS update trigger is fired when records in the COSTS table change
-- Each change of a record changes the performance of a shop, the procedure
-- proc_ShopPerformance does the processing
--
create trigger [dbo].[COSTS_TRIG_U] on [dbo].[COSTS] for UPDATE as
begin

  DECLARE crs_ShopPerform CURSOR FOR -- Get all fields from that table
    SELECT
         i.COST_ID
        ,i.SHOP_ID
        ,i.PRICE
        ,CUR.NAMEvc
        ,CTP.DESCRIPTIONvc
    FROM inserted i, deleted d, COST_TYPES CTP, CURRENCY CUR
   WHERE CUR.CURRENCY_ID = i.CURRENCY_ID
     AND i.TYPE_ID = CTP.TYPE_ID
     AND d.COST_ID = i.COST_ID
     AND(                          
             d.SHOP_ID     <> i.SHOP_ID
          OR d.TYPE_ID     <> i.TYPE_ID
          OR d.PRICE       <> i.PRICE
          OR d.CURRENCY_ID <> i.CURRENCY_ID
        )
  FOR READ ONLY

  declare
    @Error_Text      varchar(1024)
   ,@i_COST_ID       int
   ,@i_SHOP_ID       int
   ,@i_DESCRIPTIONvc varchar(64)
   ,@i_PRICE         numeric(10,2)
   ,@i_NAMEvc        varchar(64)
   
    OPEN crs_ShopPerform
    WHILE (1 = 1)
    BEGIN
      FETCH crs_ShopPerform INTO @i_COST_ID, @i_SHOP_ID, @i_PRICE, @i_NAMEvc, @i_DESCRIPTIONvc
   
      IF (@@FETCH_STATUS <> 0) BREAK -- Break loop once we finished this

      exec proc_ShopPerformance 3, 1, @i_COST_ID, @i_PRICE,@i_NAMEvc, @i_DESCRIPTIONvc,@i_SHOP_ID
    END
    CLOSE crs_ShopPerform
    DEALLOCATE crs_ShopPerform

end

GO
/****** Object:  Trigger [dbo].[PRODUCT_TRIG_D]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
--
-- The PRODUCT delete trigger is fired when records in the PRODUCT table are deleted
-- Deleting a product record can cause a PRODUCT_PRICE record's disappearance
--
create trigger [dbo].[PRODUCT_TRIG_D] on [dbo].[PRODUCT] for DELETE as
begin

DELETE PRODUCT_PRICE
  FROM inserted i, PRODUCT_PRICE PP
 WHERE PP.Product_ID = i.PRODUCT_ID

end -- End of Trigger

GO
/****** Object:  Trigger [dbo].[PRODUCT_TRIG_I]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO

--
-- The PRODUCT_TRIG_I insert trigger is fired when new records in the PRODUCT table are inserted
-- Each new PRODUCT entry causes an insert on PRODUCT_PRICE
--
create trigger [dbo].[PRODUCT_TRIG_I] on [dbo].[PRODUCT] for INSERT as
begin
INSERT INTO PRODUCT_PRICE
(
   Product_ID,NAMEvc,DESCRIPTIONvc,bVALID
  ,STDRD_PRICE_ID,STDRD_VALID_FROMdt,STDRD_VALID_TOdt,STDRD_PRICEnc,STDRD_CURRENCY_ID
  ,SALES_PRICE_ID,SALES_VALID_FROMdt,SALES_VALID_TOdt,SALES_PRICEnc,SALES_CURRENCY_ID,SALES_DESCRIPTIONvc,SALES_NAMEvc
  ,CAT_NAMEvc,CAT_SUB_NAMEvc,CAT_DESCRIPTIONvc
)
SELECT
   Product_ID    = i.PRODUCT_ID
  ,NAMEvc        = i.NAMEvc
  ,DESCRIPTIONvc = i.DESCRIPTIONvc
  ,bVALID        = i.bVALID
  -- STDRD Records
  ,STDRD_PRICE_ID     = STD_PR.PRICE_ID
  ,STDRD_VALID_FROMdt = STD_PR.VALID_FROMdt
  ,STDRD_VALID_TOdt   = STD_PR.VALID_TOdt
  ,STDRD_PRICEnc      = STD_PR.PRICEnc
  ,STDRD_CURRENCY_ID  = STD_PR.CURRENCY_ID
  -- SALES Records
  ,SALES_PRICE_ID      = SLPR.PRICE_ID
  ,SALES_VALID_FROMdt  = SLPR.VALID_FROMdt
  ,SALES_VALID_TOdt    = SLPR.VALID_TOdt
  ,SALES_PRICEnc       = SLPR.PRICEnc
  ,SALES_CURRENCY_ID   = SLPR.CURRENCY_ID
  ,SALES_DESCRIPTIONvc = SLPR.DESCRIPTIONvc
  ,SALES_NAMEvc        = SLPR.NAMEvc
  --CATEGORY_1_ID    int         not null,
  --CATEGORY_2_ID    int         not null
  ,CAT_NAMEvc          = CAT.NAMEvc
  ,CAT_SUB_NAMEvc      = CAT.SUB_NAMEvc
  ,CAT_DESCRIPTIONvc   = CAT.SUB_NAMEvc
  FROM inserted i, STANDARDPRICE STD_PR, SALESPRICE SLPR, CATEGORY CAT
 WHERE STD_PR.PRICE_ID = i.STDRD_PRICE_ID
   AND SLPR.PRICE_ID   = i.SALES_PRICE_ID
   AND CAT.CAT_1_ID    = i.CATEGORY_1_ID
   AND CAT.CAT_1_ID    = i.CATEGORY_2_ID

end -- End of Trigger

GO
/****** Object:  Trigger [dbo].[PRODUCT_TRIG_U]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO

--
-- The PRODUCT update trigger is fired when records in the PRODUCT table are updated
-- Each change in a record is processed by the proc_SalesHistory table which in
-- turn is used to analyze changing data over time ...
--
create trigger [dbo].[PRODUCT_TRIG_U] on [dbo].[PRODUCT] for UPDATE as
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
   
      IF (@@FETCH_STATUS <> 0) BREAK -- Break loop once we finished this

      exec proc_SalesHistory 3, @SALES_ID, @PURCHASE_ID, @SHOP_ID, @PRICEnc, @NAMEvc, @UNITSi
    END
    CLOSE crs_Hist
    DEALLOCATE crs_Hist

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

GO
/****** Object:  Trigger [dbo].[PRODUCT_PRICE_TRIG_D]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO

--
-- The PRODUCT_PRICE tables is a decomposition of the
-- product, StandardPrice, SalesPrice, and Category table
-- Normally, references to products are never deleted,
-- but never say never, so if a product reference is deleted an error
-- message is stored and the dba/developer is expected to patch things manually...
--
create trigger [dbo].[PRODUCT_PRICE_TRIG_D] on [dbo].[PRODUCT_PRICE] for DELETE as
begin

DECLARE cu_all CURSOR FOR -- Get all fields from that table
  SELECT d.Product_ID
        ,d.NAMEvc
        ,d.DESCRIPTIONvc
        ,d.bValid
    FROM deleted d
FOR READ ONLY
 
declare
   @PARAMSvc        varchar(1024)
  ,@d_Product_ID    int
  ,@d_NAMEvc        varchar(64)
  ,@d_DESCRIPTIONvc varchar(255)
  ,@d_bValid        bit
  
  OPEN cu_all
  WHILE (1 = 1)
  BEGIN
    FETCH cu_all INTO @d_Product_ID ,@d_NAMEvc ,@d_DESCRIPTIONvc ,@d_bValid
 
    IF (@@FETCH_STATUS <> 0) BREAK
 
    select @PARAMSvc = 
        ' Product_ID=' + convert(varchar, @d_Product_ID)
      + ',NAMEvc=' + convert(varchar, @d_NAMEvc)
      + ',DESCRIPTIONvc=' + convert(varchar, @d_DESCRIPTIONvc)
      + ',bValid=' + convert(varchar, @d_bValid)

    exec proc_Error "PRODUCT_PRICE_TRIG_D", @PARAMSvc, "ERROR: PRODUCT ENTRY PHYSICALLY DELETED!"
  END
  CLOSE cu_all
  DEALLOCATE cu_all
end -- End of Trigger

GO
/****** Object:  Trigger [dbo].[PURCHASE_TRIG_D]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
--
-- The Purchase delete trigger is fired when records in the COSTS table are deleted
-- Each deleted record changes the Purchase-History of a shop, the procedure
-- proc_PurchaseHistory does the processing
--
create trigger [dbo].[PURCHASE_TRIG_D] on [dbo].[PURCHASE] for DELETE as
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
 
    IF (@@FETCH_STATUS <> 0) BREAK
 
    exec proc_PurchaseHistory 1, @d_PURCHASE_ID, @d_PURCHASE_PRICEnc, @CUR_NAMEvc, @d_UNITSi
  END
  CLOSE crs_PurchHist
  DEALLOCATE crs_PurchHist

end

GO
/****** Object:  Trigger [dbo].[PURCHASE_TRIG_I]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO

--
-- The Purchase insert trigger is fired when records in the COSTS table are inserted
-- Each new purchase changes the Purchase-History of a shop, the procedure
-- proc_PurchaseHistory does the processing
--
create trigger [dbo].[PURCHASE_TRIG_I] on [dbo].[PURCHASE] for INSERT as
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
 
    IF (@@FETCH_STATUS <> 0) BREAK
 
    exec proc_PurchaseHistory 2, 2, @i_PURCHASE_ID, @i_PURCHASE_PRICEnc, @CUR_NAMEvc, @i_UNITSi
  END
  CLOSE crs_PurchHist
  DEALLOCATE crs_PurchHist

end

GO
/****** Object:  Trigger [dbo].[SALES_TRIG_U]    Script Date: 06.05.2015 21:24:39 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO

--
-- The sales update trigger is fired when records in the SALES table are updated
-- Each change in a record is processed by the proc_SalesHistory table which in
-- turn is used to analyze changing data over time ...
--
create trigger [dbo].[SALES_TRIG_U] on [dbo].[SALES] for UPDATE as
begin

  DECLARE crs_Hist CURSOR FOR -- Get all fields from that table
  SELECT i.SALES_ID
        ,i.PURCHASE_ID                     
        ,i.SHOP_ID                         
        ,i.PRICEnc
        ,CUR.NAMEvc
        ,i.UNITSi
        FROM inserted i, deleted d, CURRENCY CUR, PURCHASE PUR, PRODUCT PROD
       WHERE CUR.CURRENCY_ID = i.CURRENCY_ID
         AND PUR.PURCHASE_ID = i.PURCHASE_ID
         AND PROD.PRODUCT_ID = PUR.PRODUCT_ID
         AND PROD.bVALID     = 1               -- We want only valid entries
         AND d.SALES_ID      = i.SALES_ID
         AND( d.PURCHASE_ID                      <> i.PURCHASE_ID                     
           OR d.SHOP_ID                          <> i.SHOP_ID                         
           OR d.PRICEnc                          <> i.PRICEnc                         
           OR d.CURRENCY_ID                      <> i.CURRENCY_ID                     
           OR d.UNITSi                           <> i.UNITSi                          
            )
  FOR READ ONLY
   
  declare
     @Error_Text varchar(1024)
    ,@i_SALES_ID    int
    ,@i_PURCHASE_ID int
    ,@i_SHOP_ID     int
    ,@i_PRICEnc     numeric(10,2)
    ,@NAMEvc        varchar(64)
    ,@i_UNITSi      int
   
    OPEN crs_Hist
    WHILE (1 = 1)
    BEGIN
      FETCH crs_Hist INTO  @i_SALES_ID, @i_PURCHASE_ID, @i_SHOP_ID, @i_PRICEnc, @NAMEvc, @i_UNITSi
   
      IF (@@FETCH_STATUS <> 0) BREAK -- Break loop once we finished this

      exec proc_SalesHistory 3, @i_SALES_ID, @i_PURCHASE_ID, @i_SHOP_ID, @i_PRICEnc, @NAMEvc, @i_UNITSi

      exec proc_ShopPerformance 3, 2, @i_PURCHASE_ID, @i_PRICEnc,@NAMEvc, '',@i_SHOP_ID

    END
    CLOSE crs_Hist
    DEALLOCATE crs_Hist

end -- End of Trigger

GO
USE [master]
GO
ALTER DATABASE [TITAN] SET  READ_WRITE 
GO
