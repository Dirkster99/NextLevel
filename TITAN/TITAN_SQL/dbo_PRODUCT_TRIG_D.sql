
--
-- The PRODUCT delete trigger is fired when records in the PRODUCT table are deleted
-- Deleting a product record can cause a PRODUCT_PRICE record's disappearance
--
create trigger PRODUCT_TRIG_D on PRODUCT for DELETE as
begin

DELETE PRODUCT_PRICE
  FROM inserted i, PRODUCT_PRICE PP
 WHERE PP.Product_ID = i.PRODUCT_ID

end -- End of Trigger
