
/*************************************
Create the product table which contains
information about every product being
purchesed from different vendors and
sold in various shops to costumers
*/
create table PRODUCT
(
  Product_ID       int          not null,
  NAMEvc           varchar(64)  not null,
  DESCRIPTIONvc    varchar(255) not null,

  STDRD_PRICE_ID   int         not null,
  SALES_PRICE_ID   int         not null,

  CATEGORY_1_ID    int         not null,
  CATEGORY_2_ID    int         not null,

  bVALID           bit         not null
)
go

create unique index product_id_ind on PRODUCT(PRODUCT_ID)

go

/*************************************
Create the purchase table which contains
information about every purchase being
conducted at different vendors to sell
these products to the consumer
*/
create table PURCHASE
(
  PURCHASE_ID      int           not null,
  VENDOR_ID        int           not null,
  PRODUCT_ID       int           not null,
  PURCHASE_PRICEnc numeric(10,2) not null,
  CURRENCY_ID      int           not null,
  UNITSi           int           not null
)
go

create unique index purchase_id_ind on PURCHASE(PURCHASE_ID)
go

/*******************************************
Create a vendor table containing information
about the vendors of different products, which
are sold in world wide shops to customers.
*/
create table VENDOR
(
  VENDOR_ID        int           not null,
  NAMEvc           varchar(64)   not null,
  COUNTRYvc        varchar(64)   not null
)
go

create unique index vendor_id_ind on VENDOR(VENDOR_ID)
go

/*******************************************
Create a StandardPrice table containing
information about the prices that are
usually offered to costumers for certain
products.
*/
create table STANDARDPRICE
(
  PRICE_ID         int           not null,
  VALID_FROMdt     datetime      not null,
  VALID_TOdt       datetime      not null,

  PRICEnc          numeric(10,2) not null,
  CURRENCY_ID      varchar(64)   not null
)
go

create unique index standard_id_ind on STANDARDPRICE(PRICE_ID)
go

/*******************************************
Create a CURRENCY table which contains
information about every CURRENCY being
handled withint this system.
*/
create table CURRENCY
(
  CURRENCY_ID      int           not null,
  NAMEvc           varchar(64)   not null
)
go

create unique index currency_id_ind on CURRENCY(CURRENCY_ID)
go

/*******************************************
Create a CURRENCY table which contains
information about every CURRENCY being
handled withint this system.
*/
create table SALESPRICE
(
  PRICE_ID         int           not null,
  VALID_FROMdt     datetime      not null,
  VALID_TOdt       datetime      not null,

  PRICEnc          numeric(10,2) not null,
  CURRENCY_ID      int           not null,
  DESCRIPTIONvc    varchar(255)  not null,
  NAMEvc           varchar(64)   not null
)
go

create unique index salesprice_id_ind on SALESPRICE(PRICE_ID)
go

/*******************************************
Create a SHOP table which contains
information about every Shop tha is
part of the TITAN company. Shops sell
products or parts to customers and can
be physically located at a certain address
(on the web or elsewhere).
*/
create table SHOP
(
	SHOP_ID          int           not null,
  ADDRESS_ID       int           not null,
  NAMEvc           varchar(64)   not null,
  TYPE             VARCHAR(64)   not null
)
go

create unique index shop_id_ind on SHOP(SHOP_ID)
go

/*******************************************
Create an ADDRESS table which contains
contact information about shop that is
part of the company.
*/
create table ADDRESS
(
  ADDRESS_ID       int           not null,

  COUNTRYvc        varchar(64)   not null,
  CITYvc           varchar(64)   not null,
  ZIPCODEvc        varchar(64)   not null,
  STREETvc         varchar(64)   not null,
  STATEvc          varchar(64)   not null,

  EMAILvc          varchar(255)  not null,
  WEB_SITEvc       varchar(255)  not null
)
go

create unique index ADDRESS_ID_ind on ADDRESS(ADDRESS_ID)
go

/*******************************************
Create a SALES table which contains
information about products that were
sold for a certain price in a certain shop.
*/
create table SALES
(
  SALES_ID         int           not null,
  PURCHASE_ID      int           not null,
  SHOP_ID          int           not null,

  PRICEnc          numeric(10,2) not null,
  CURRENCY_ID      int           not null,
  UNITSi           int           not null,
)
go

create unique index SALES_ID_ind on SALES(SALES_ID)
go

/*******************************************
Create a CATEGORY table which contains
categories of products. This information
is used to help customers and staff searching
for information about certain products
in certain groups of items.
*/
create table CATEGORY
(
  CAT_1_ID         int           not null,
  CAT_2_ID         int           not null,
  NAMEvc           varchar(64)   not null,
  SUB_NAMEvc       varchar(64)   not null,
  DESCRIPTIONvc    varchar(64)   not null
)
go

create unique index CATEGORY_ind on CATEGORY(CAT_1_ID, CAT_2_ID)
go

/*******************************************
Create a COSTS table which contains
information about fixed costs that
shops have in order to run their facilities.
This costs (can) include staff salaries,
storage rents, and other types of fixed costs.
*/
create table COSTS
(
  COST_ID          int           not null,
  SHOP_ID          int           not null,
  TYPE_ID          int           not null,

  PRICE            numeric(10,2) not null,
  CURRENCY_ID      int           not null,
  DateStamp        datetime      not null
)
go

create unique index COSTS_ID_ind on COSTS(COST_ID)
go

/*******************************************
Create a COST_TYPE table which contains
information about the different types of
fixed costs.
*/
create table COST_TYPES
(
  TYPE_ID          int           not null,
  DESCRIPTIONvc    varchar(64)   not null,
)
go

create unique index COST_TYPES_ID_ind on COST_TYPES(TYPE_ID)
go

/*******************************************
Create a ERROR_MSG table which contains
information about errors that were logged
during the livetime of the dbms system
*/
create table ERROR_MSG
(
  ID        numeric(10,0) identity
 ,NOWdt     datetime
 ,SOURCEvc  VARCHAR(32)
 ,PARAMSvc  VARCHAR(255)
 ,MESSAGEvc VARCHAR(255)
)
go

create unique index ERROR_MSG_ID_ind on ERROR_MSG(ID)
go

-- XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
-- Datawarehouse tables
-- XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
--
-- Table Shop_Performance is used derive the performance
-- of shop, which is a comparison of costs and sales of a shop
--
create table Shop_Performance
(
  SP_ID        numeric(10,0) identity
 ,SHOP_ID      int          not null
 ,SHOP_NAME    VARCHAR(32)  not null
 ,SHOP_TYPE    VARCHAR(255) not null

 ,SHOP_ADDRESS_ID       int           not null
 ,SHOP_COUNTRYvc        varchar(64)   not null
 ,SHOP_CITYvc           varchar(64)   not null
 ,SHOP_ZIPCODEvc        varchar(64)   not null
 ,SHOP_STREETvc         varchar(64)   not null
 ,SHOP_STATEvc          varchar(64)   not null
 ,SHOP_EMAILvc          varchar(255)  not null
 ,SHOP_WEB_SITEvc       varchar(255)  not null

 ,COST_ID               int               null
 ,PURCHASE_ID           int               null
 ,PRICE_TYPE            int           not null -- 1 for COSTS, 2 for SALES
 ,PRICE                 numeric(10,2) not null -- price in EURO can be cost or sales
 ,PRICE_DESCRIPTION     varchar(255)  not null

 ,VERSION              int           not null
 ,V_FROM               datetime      not null
 ,V_TO                 datetime          null
)
go

create unique index Shop_Performance_ID_ind on Shop_Performance(SP_ID)
go

create unique index Shop_Performance_VERS_ind on Shop_Performance(VERSION, V_FROM, V_TO)
on indexsegment
go

--
-- Table SALES_HISTORY shows how a shop's sales history
-- changes over time, reports can also analyze dependence
-- of vendors, units and localization
-- In other words, who's products sold best? What was its category? etc...
--
create table SALES_HISTORY
(
  SH_ID        numeric(10,0) identity
 ,SALES_ID     int          not null

 ,SALES_PRICE numeric(10,2) not null -- price in EURO of sales
 ,SALES_UNITS  int          not null

 ,VENDOR_ID       int           not null
 ,VENDOR_NAME     varchar(64)   not null
 ,VENDOR_COUNTRY  varchar(64)   not null

 ,PRODUCT_ID          int           not null
 ,PRODUCT_NAME        varchar(64)   not null
 ,PRODUCT_DESCRIPTION varchar(64)   not null

 ,CATEGORY_NAME        varchar(64)   not null
 ,CATEGORY_SUB_NAME    varchar(64)   not null

 ,SHOP_ID      int          not null
 ,SHOP_NAME    VARCHAR(32)  not null
 ,SHOP_TYPE    VARCHAR(255) not null

 ,SHOP_ADDRESS_ID       int           not null
 ,SHOP_COUNTRYvc        varchar(64)   not null
 ,SHOP_CITYvc           varchar(64)   not null
 ,SHOP_ZIPCODEvc        varchar(64)   not null
 ,SHOP_STREETvc         varchar(64)   not null
 ,SHOP_STATEvc          varchar(64)   not null
 ,SHOP_EMAILvc          varchar(255)  not null
 ,SHOP_WEB_SITEvc       varchar(255)  not null

 ,VERSION              int           not null
 ,V_FROM               datetime      not null
 ,V_TO                 datetime          null
)
go

create unique index SALES_HISTORY_ID_ind on SALES_HISTORY(SH_ID)
go

create unique index SALES_HISTORY_VERS_ind on SALES_HISTORY(VERSION, V_FROM, V_TO)
on indexsegment
go


--
-- Table PURCHASE_HISTORY shows who s te best vendor
-- best on price per untis for products of a certain category
--
create table PURCHASE_HISTORY
(
  PH_ID        numeric(10,0) identity
 ,PURCHASE_ID  int          not null

 ,PURCHASE_PRICE numeric(10,2) not null -- price in EURO of sales
 ,PURCHASE_UNITS  int          not null

 ,VENDOR_ID       int           not null
 ,VENDOR_NAME     varchar(64)   not null
 ,VENDOR_COUNTRY  varchar(64)   not null

 ,PRODUCT_ID          int           not null
 ,PRODUCT_NAME        varchar(64)   not null
 ,PRODUCT_DESCRIPTION varchar(64)   not null

 ,CATEGORY_NAME        varchar(64)   not null
 ,CATEGORY_SUB_NAME    varchar(64)   not null

 ,VERSION              int           not null
 ,V_FROM               datetime      not null
 ,V_TO                 datetime          null
)
go

create unique index PURCHASE_HISTORY_ID_ind on PURCHASE_HISTORY(PH_ID)
go

create unique index PURCHASE_HISTORY_VERS_ind on PURCHASE_HISTORY(VERSION, V_FROM, V_TO)
on indexsegment
go

-- XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

/****************************************************
Create the product_price table which is an aggregate
of the product, Category, StandardPrice,
 and SalesPrice tables
*/
create table PRODUCT_PRICE
(
   Product_ID       int          not null
  ,NAMEvc           varchar(64)  not null
  ,DESCRIPTIONvc    varchar(255) not null
  ,bVALID           bit          not null

  -- STDRD Records
  ,STDRD_PRICE_ID         int      not null
  ,STDRD_VALID_FROMdt     datetime not null
  ,STDRD_VALID_TOdt       datetime not null
  ,STDRD_PRICEnc          numeric(10,2) not null
  ,STDRD_CURRENCY_ID      varchar(64)   not null

  -- SALES Records
  ,SALES_PRICE_ID         int           not null
  ,SALES_VALID_FROMdt     datetime      not null
  ,SALES_VALID_TOdt       datetime      not null
  ,SALES_PRICEnc          numeric(10,2) not null
  ,SALES_CURRENCY_ID      int           not null
  ,SALES_DESCRIPTIONvc    varchar(255)  not null
  ,SALES_NAMEvc           varchar(64)   not null

  --CATEGORY_1_ID    int         not null,
  --CATEGORY_2_ID    int         not null
  ,CAT_NAMEvc           varchar(64)   not null  -- Fields from Category table
  ,CAT_SUB_NAMEvc       varchar(64)   not null
  ,CAT_DESCRIPTIONvc    varchar(64)   not null
)
go

create unique index PRODUCT_PRICE_id_ind on PRODUCT_PRICE(PRODUCT_ID)
