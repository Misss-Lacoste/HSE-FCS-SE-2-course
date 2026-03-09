--SELECT * FROM SOTRUDNIKI t
--where t.secondname is NULL
--order by t.id fetch first 3 rows only
--select * from sotrudniki t where t.id between 1 and 5 order by t.id asc
--where t.surname = '&var'
--SELECT t.*, to_char (t.birth, 'fmDD "of" Month, YYYY') "Дата Рождения" from sotrudniki t
/*SELECT t.*,
       to_char(t.birth, 'dd.mm.yyyy') "1",
       to_char(t.birth, 'dd.mm.yyyy hh24:mi:ss') "2",
       to_char(t.birth, 'fmDD Month YYYY') "3",
       to_char(t.birth, 'fmDth "of" Month, YYYY') "4",
       to_char(t.birth, 'DY') "5",
       to_char(t.birth, 'Day') "6",
       to_char(t.birth, 'DDSPTH') "7"
  from sotrudniki t;*/
--SELECT t.* from sotrudniki t where to_char(t.birth,'yyyy')='1985'
SELECT t.*,
CASE WHEN t.secondname is NULL then 'нет'
ELSE t.secondname end
from sotrudniki t;