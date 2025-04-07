# Informace o úloze #
Tato úloha je specificky na multithreading v jazyce C++. Jedná se o těžší verzi problému producent/konzument.
Zadání je, že náš program je určen pro svářecí společnost (CWeldingCompany) prodávající plechy (COrder) a máme být schopni obsluhovat několik zákazníků (CCustomer) najednou, získávat od nich poptávky (COrderList) na určitý typ materiálu(material_id), nacenit požadované plechy v nich pomocí ceníků (CPriceList) od dodavatelů (CProducer) a následně jim poptávky vracet. 
Plechy nelze řezat a lze je pouze skládat a svářet k sobě. Svářet navíc lze pouze plechy, jejichž svářené hrany jsou stejně dlouhé. Například: máme plechy ve formátu <šířka> x <výška>, k dispozici máme plechy 3x4, 3x6 a 4x9. Nejdříve lze svařit k sobě plechy 3x4 a 3x6 a vytvořit tak plech o velikosti 3x9, protože mají stejnou šířku. Tento plech lze pak svařit s plechem o velikosti 4x9 a vytvořit tak plech o velikosti 7x9, protože mají stejnou výšku.

