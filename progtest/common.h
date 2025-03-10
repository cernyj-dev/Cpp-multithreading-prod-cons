// The classes in this header define the common interface between your implementation and
// the testing environment. Exactly the same implementation is present in the progtest's
// testing environment. You are not supposed to modify any declaration in this file,
// any change is likely to break the compilation.
#ifndef COMMON_H_09824352756248526345245
#define COMMON_H_09824352756248526345245

#include <vector>
#include <memory>
//=============================================================================================================================================================
class CProd{
  public:
    unsigned m_W;
    unsigned m_H;
    double m_Cost;

    CProd(unsigned w, unsigned h, double cost): m_W(w), m_H(h), m_Cost(cost){}
};
//=============================================================================================================================================================
class CPriceList{
  public:
    unsigned m_MaterialID;
    std::vector<CProd> m_List;

    CPriceList(unsigned materialID): m_MaterialID(materialID){}

    CPriceList* add(const CProd& x){
      m_List.push_back(x);
      return this;
    }

    virtual ~CPriceList() = default;
};
using APriceList = std::shared_ptr<CPriceList>;
//=============================================================================================================================================================
class COrder{
  public:
    unsigned m_W;
    unsigned m_H;
    double m_WeldingStrength;
    double m_Cost = 0;

    COrder(unsigned w, unsigned h, double weldingStrength): m_W(w), m_H (h), m_WeldingStrength(weldingStrength){}
};
//=============================================================================================================================================================
class COrderList
{
  public:
    unsigned m_MaterialID;
    std::vector<COrder> m_List;
  
    COrderList(unsigned materialID): m_MaterialID(materialID){}

    COrderList* add(const COrder& x){
      m_List.push_back(x);
      return this;
    }

    virtual ~COrderList() = default;
};
using AOrderList = std::shared_ptr<COrderList>;
//=============================================================================================================================================================
class CProducer : public std::enable_shared_from_this<CProducer>{
  public:
    virtual void sendPriceList(unsigned materialID) = 0;

    virtual ~CProducer() = default;
};
using AProducer = std::shared_ptr<CProducer>;
//=============================================================================================================================================================
class CCustomer : public std::enable_shared_from_this<CCustomer>{
  public:
    virtual AOrderList waitForDemand() = 0;

    virtual void completed(AOrderList x) = 0;
    
    virtual ~CCustomer() = default;
};
using ACustomer = std::shared_ptr<CCustomer>;
//=============================================================================================================================================================
#endif /* COMMON_H_09824352756248526345245 */
