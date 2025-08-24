
#include "OrderBookResting.h"
#include <cmath>

typedef std::vector<std::tuple<float, PLSimpleList > > InnerBookPrintList;
typedef std::map<std::string, InnerBookPrintList> OBPrint;
typedef std::vector<std::tuple<int, int, float, int> > Ledger;

typedef std::tuple<float, int, int> MMTransaction;

class OrderBookMatching : public OrderBook {
private:
    Ledger ledger;

public:
    OBPrint getPrintBook();
    std::string printBook();

    InnerBookPrintList getPrintAsks();
    InnerBookPrintList getPrintBids();
    void addOrder(int orderID, std::string side, float price, int qty, std::string type);

    MMTransaction popBids();
    MMTransaction popAsks();
    MMTransaction partialBuy(int amt);
    MMTransaction partialSell(int amt);

    void addTransaction(int takerID, int makerID, float price, int qty);

    Ledger getLedger() const;
};