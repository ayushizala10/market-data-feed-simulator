#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;
using namespace chrono;

struct MarketEvent {
    uint64_t eventId;
    string symbol;
    double price;
    int quantity;
};

struct StockState {
    double latestPrice = 0.0;
    double firstPrice = 0.0;
    long long totalVolume = 0;
    uint64_t updateCount = 0;
};

class MarketDataFeed {
private:
    unordered_map<string, StockState> market;
    uint64_t lastEventId = 0;
    uint64_t processedEvents = 0;
    long long totalVolume = 0;

public:
    bool processEvent(const MarketEvent& event) {
        if (event.eventId <= lastEventId ||
            event.price <= 0 ||
            !isfinite(event.price) ||
            event.quantity <= 0 ||
            event.symbol.empty()) {
            return false;
        }

        auto& stock = market[event.symbol];

        if (stock.updateCount == 0) {
            stock.firstPrice = event.price;
        }

        stock.latestPrice = event.price;
        stock.totalVolume += event.quantity;
        stock.updateCount++;

        totalVolume += event.quantity;
        lastEventId = event.eventId;
        processedEvents++;

        return true;
    }

    void displayMarket() const {
        cout << "\n========== MARKET SNAPSHOT ==========\n";
        cout << left << setw(12) << "Symbol"
             << right << setw(12) << "Latest Price"
             << setw(15) << "Total Volume"
             << setw(12) << "Updates"
             << setw(14) << "Change (%)" << '\n';

        cout << string(65, '-') << '\n';

        vector<string> symbols;
        for (const auto& entry : market) {
            symbols.push_back(entry.first);
        }
        sort(symbols.begin(), symbols.end());

        for (const auto& symbol : symbols) {
            const auto& stock = market.at(symbol);

            double change = 0.0;
            if (stock.firstPrice != 0.0) {
                change = (stock.latestPrice - stock.firstPrice)
                         / stock.firstPrice * 100.0;
            }

            cout << left << setw(12) << symbol
                 << right << fixed << setprecision(2)
                 << setw(12) << stock.latestPrice
                 << setw(15) << stock.totalVolume
                 << setw(12) << stock.updateCount
                 << setw(13) << change << "%" << '\n';
        }

        cout << "\nEvents processed: " << processedEvents
             << "\nTotal volume: " << totalVolume << '\n';
        cout << "=====================================\n";
    }

    void displayStock(const string& symbol) const {
        auto it = market.find(symbol);

        if (it == market.end()) {
            cout << "No data found for " << symbol << ".\n";
            return;
        }

        const auto& stock = it->second;

        double change = (stock.latestPrice - stock.firstPrice)
                        / stock.firstPrice * 100.0;

        cout << "\nSymbol: " << symbol
             << "\nFirst price: " << fixed << setprecision(2)
             << stock.firstPrice
             << "\nLatest price: " << stock.latestPrice
             << "\nTotal volume: " << stock.totalVolume
             << "\nNumber of updates: " << stock.updateCount
             << "\nPrice change: " << change << "%\n";
    }

    void generateSampleFeed(int count) {
        const vector<string> symbols = {
            "AAPL", "MSFT", "GOOG", "AMZN", "NVDA"
        };

        mt19937 rng(42);
        uniform_int_distribution<int> symbolDist(
            0, static_cast<int>(symbols.size()) - 1
        );
        uniform_int_distribution<int> priceDist(-200, 200);
        uniform_int_distribution<int> quantityDist(1, 1000);

        uint64_t nextId = lastEventId + 1;

        for (int i = 0; i < count; ++i) {
            string symbol = symbols[symbolDist(rng)];

            double basePrice;
            if (symbol == "AAPL") basePrice = 225.0;
            else if (symbol == "MSFT") basePrice = 500.0;
            else if (symbol == "GOOG") basePrice = 175.0;
            else if (symbol == "AMZN") basePrice = 220.0;
            else basePrice = 140.0;

            double price = basePrice + priceDist(rng) / 100.0;
            int quantity = quantityDist(rng);

            processEvent({nextId++, symbol, price, quantity});
        }

        cout << count << " sample events generated and processed.\n";
    }

    void benchmark(int count) {
        if (count <= 0) {
            cout << "Count must be positive.\n";
            return;
        }

        vector<MarketEvent> events;
        events.reserve(count);

        mt19937 rng(123);
        uniform_int_distribution<int> symbolDist(0, 4);
        uniform_int_distribution<int> priceDist(-200, 200);
        uniform_int_distribution<int> quantityDist(1, 1000);

        const vector<string> symbols = {
            "AAPL", "MSFT", "GOOG", "AMZN", "NVDA"
        };

        const double basePrices[] = {
            225.0, 500.0, 175.0, 220.0, 140.0
        };

        for (int i = 0; i < count; ++i) {
            int index = symbolDist(rng);
            events.push_back({
                static_cast<uint64_t>(i + 1),
                symbols[index],
                basePrices[index] + priceDist(rng) / 100.0,
                quantityDist(rng)
            });
        }

        MarketDataFeed testFeed;
        vector<long long> latencies;
        latencies.reserve(count);

        auto totalStart = steady_clock::now();

        for (const auto& event : events) {
            auto start = steady_clock::now();
            testFeed.processEvent(event);
            auto finish = steady_clock::now();

            latencies.push_back(
                duration_cast<nanoseconds>(finish - start).count()
            );
        }

        auto totalFinish = steady_clock::now();

        sort(latencies.begin(), latencies.end());

        long long latencySum = 0;
        for (long long value : latencies) {
            latencySum += value;
        }

        double elapsedSeconds =
            duration<double>(totalFinish - totalStart).count();

        cout << fixed << setprecision(2);
        cout << "\n========== FEED BENCHMARK ==========\n";
        cout << "Events processed: " << testFeed.processedEvents << '\n';
        cout << "Average latency: "
             << static_cast<double>(latencySum) / count << " ns\n";
        cout << "Median latency (p50): "
             << latencies[(count - 1) * 50 / 100] << " ns\n";
        cout << "p95 latency: "
             << latencies[(count - 1) * 95 / 100] << " ns\n";
        cout << "p99 latency: "
             << latencies[(count - 1) * 99 / 100] << " ns\n";
        cout << "Throughput: "
             << count / elapsedSeconds << " events/second\n";
        cout << "====================================\n";
    }

    bool saveSnapshot(const string& filename) const {
        ofstream file(filename);

        if (!file) {
            return false;
        }

        file << "Symbol,FirstPrice,LatestPrice,TotalVolume,Updates\n";

        vector<string> symbols;
        for (const auto& entry : market) {
            symbols.push_back(entry.first);
        }
        sort(symbols.begin(), symbols.end());

        for (const auto& symbol : symbols) {
            const auto& stock = market.at(symbol);
            file << symbol << ','
                 << stock.firstPrice << ','
                 << stock.latestPrice << ','
                 << stock.totalVolume << ','
                 << stock.updateCount << '\n';
        }

        return static_cast<bool>(file);
    }
};

int main() {
    MarketDataFeed feed;

    while (true) {
        cout << "\n====== MARKET DATA FEED SIMULATOR ======\n";
        cout << "1. Generate 100 sample events\n";
        cout << "2. Generate 1,000 sample events\n";
        cout << "3. Generate 100,000 sample events\n";
        cout << "4. Display market snapshot\n";
        cout << "5. View a stock\n";
        cout << "6. Run performance benchmark\n";
        cout << "7. Export market snapshot to CSV\n";
        cout << "8. Exit\n";
        cout << "Enter choice: ";

        int choice;
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Please enter a valid number.\n";
            continue;
        }

        if (choice == 1) {
            feed.generateSampleFeed(100);
        } else if (choice == 2) {
            feed.generateSampleFeed(1000);
        } else if (choice == 3) {
            feed.generateSampleFeed(100000);
        } else if (choice == 4) {
            feed.displayMarket();
        } else if (choice == 5) {
            string symbol;
            cout << "Enter symbol (e.g. AAPL): ";
            cin >> symbol;
            feed.displayStock(symbol);
        } else if (choice == 6) {
            int count;
            cout << "Number of events to benchmark: ";
            if (!(cin >> count)) {
                cin.clear();
                cin.ignore(10000, '\n');
                cout << "Invalid count.\n";
                continue;
            }
            feed.benchmark(count);
        } else if (choice == 7) {
            if (feed.saveSnapshot("market_snapshot.csv")) {
                cout << "Snapshot saved to market_snapshot.csv\n";
            } else {
                cout << "Could not save the CSV file.\n";
            }
        } else if (choice == 8) {
            cout << "Goodbye!\n";
            break;
        } else {
            cout << "Invalid choice. Select 1-8.\n";
        }
    }

    return 0;
}