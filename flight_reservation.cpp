#include<bits/stdc++.h>
#include <fstream>
#include <sstream>
#include <random>
using namespace std;

struct Flight {
    string flightNumber;
    string from;
    string to;
    string date;
    double fare;
    double duration; 
    int count;
};

struct Booking {
    string flightNumber;
    string name;
    string contact;
    double fare;
    string pnr;
};

vector<Flight> flights;
vector<Booking> bookings;
// Graph source->(destination, (fare, duration))
unordered_map<string, vector<pair<string, pair<double, double>>>> flightGraph;

// Function prototypes
void loadFlights(const string& filename);
void displayFlights(const string& from, const string& to, const string& date);
double dynamicFare(const Flight& flight);//work in progress
void searchFlights(const string& from, const string& to, const string& date);
void optimizeRoute(const string& from, const string& to, const string& date, bool minimizeFare);
void showBookings();
void buildGraph();
void dijkstra(const string& start, const string& end, bool minimizeFare);
void mainMenu();
void loadBookings();
void bookFlight(const string& flightNumber, const string& name, const string& contact); //update in file is pending
void cancelBooking(); //updation in file(data.txt) is pending
void saveBooking(const Booking& booking);
string generatePNR(); 
void updateFlightCount(const string& flightNumber, int change);// work in progress
void saveBookingsToFile();
// Main function
int main() {
    loadFlights("data.txt");
    // Build the graph from the loaded flights
    buildGraph(); 
    // Load existing bookings
    loadBookings(); 
    mainMenu();
    return 0;
}

// Load flights from a file
void loadFlights(const string& filename) {
    ifstream file(filename);
    if (!file) {
        cout << "Could not open the file!" << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        Flight flight;
        ss >> flight.flightNumber >> flight.from >> flight.to >> flight.date >> flight.fare >> flight.duration >> flight.count;
        flights.push_back(flight);
    }
    file.close();
}

// Build the flight graph
void buildGraph() {
    for (const auto& flight : flights) {
        flightGraph[flight.from].push_back({flight.to, {flight.fare, flight.duration}});
    }
}

// Load existing bookings from a file 
void loadBookings() {
    ifstream file("booking.txt");
    if (!file) {
        cout << "Could not open booking file!" << endl;
        return;
    }
    Booking booking;
    while (file >> booking.flightNumber >> booking.name >> booking.contact >> booking.fare >> booking.pnr) {
        bookings.push_back(booking);
    }

    file.close();
}

string generatePNR() {
    const int PNR_LENGTH = 6;
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    string pnr;
    random_device rd;
    mt19937 generator(rd());
    uniform_int_distribution<> dist(0, sizeof(charset) - 2);

    for (int i = 0; i < PNR_LENGTH; ++i) {
        pnr += charset[dist(generator)];
    }

    return pnr;
}

// Display available flights
void displayFlights(const string& from, const string& to, const string& date) {
    bool directFound = false;
    bool indirectFound = false;

    cout << "Direct Flights:\n--------------------------\n";
    for (const auto& flight : flights) {
        if (flight.from == from && flight.to == to && flight.date == date) {
            cout << "Flight Number: " << flight.flightNumber << ", Fare: $" << flight.fare 
                 << ", Duration: " << flight.duration << " hours, Seats: " << flight.count << endl;
            directFound = true;
        }
    }

    if (!directFound) {
        cout << "No direct flights found from " << from << " to " << to << " on " << date << "." << endl;
    }

    //Handles only 1 stop indirect flights
    cout << "\nIndirect Flights:\n--------------------------\n";
    for (const auto& flight : flights) {
        if (flight.from == from && flight.date == date) {
            for (const auto& indirect : flights) {
                if (indirect.from == flight.to && indirect.to == to && indirect.date == date) {
                    cout << "Flight Number: " << flight.flightNumber << " -> " << indirect.flightNumber <<", "
                         << from <<"->"<< flight.to << "->"<< to
                         << ", Total Fare: $" << (flight.fare + indirect.fare) 
                         << ", Total Duration: " << (flight.duration + indirect.duration) << " hours." << endl;
                    indirectFound = true;
                }
            }
        }
    }
    if (!indirectFound) {
        cout << "No indirect flights found from " << from << " to " << to << " on " << date << "." << endl;
    }
}

// Book a flight and save booking details
void bookFlight(const string& flightNumber, const string& name, const string& contact) {
    for (auto& flight : flights) {
        if (flight.flightNumber == flightNumber && flight.count > 0) { // Check for available seats
            Booking booking;
            booking.flightNumber = flightNumber;
            booking.name = name;
            booking.contact = contact;
            booking.fare = flight.fare;
            booking.pnr = generatePNR();
            cout << "--------------------------" << endl;
            cout << "Booking confirmed!" << endl;
            cout << "Flight Number: " << booking.flightNumber << endl;
            cout << "Passenger Name: " << booking.name << endl;
            cout << "Contact: " << booking.contact << endl;
            cout << "Total Fare: $" << booking.fare << endl;
            cout << "PNR: " << booking.pnr << endl; 
            cout << "--------------------------" << endl;
            saveBooking(booking);
            bookings.push_back(booking); // Store booking in global vector
            // updateFlightCount(flightNumber, -1); // Decrement seat count
            return;
        }
    }
    cout << "Flight not found " << endl;
}

// double dynamicFare(const Flight& flight) {
//   
// }

// Save booking details to a file
void saveBooking(const Booking& booking) {
    ofstream file("booking.txt", ios::app);
    if (file) {
        file << booking.flightNumber << " " << booking.name << " " << booking.contact 
             << " " << booking.fare << " " << booking.pnr << endl; // Save PNR
        file.close();
    } else {
        cout << "Could not open booking file!" << endl;
    }
}

// Search for flights
void searchFlights(const string& from, const string& to, const string& date) {
    displayFlights(from, to, date);
}

// Optimize route for minimum fare or time
void optimizeRoute(const string& from, const string& to, const string& date, bool minimizeFare) {
    cout << "Optimizing route from " << from << " to " << to << " on " << date << endl;
    dijkstra(from, to, minimizeFare);
}

// dijkstra's algorithm with multiple stops 
void dijkstra(const string& start, const string& end, bool minimizeFare) {

    // to store the cost to each destination
    unordered_map<string, double> dist;  

    // to store path
    unordered_map<string, vector<string>> prev; 

    // to track the total duration 
    unordered_map<string, double> durationMap;

    // priority queue for selecting the next flight 
    set<pair<double, string>> pq; 
    // priority_queue<pair<int,int>,vector<pair<int,int>>,greater<pair<int,int>>>pq;
    for (const auto& flight : flights) {
        dist[flight.from] = LONG_LONG_MAX;
        dist[flight.to] = LONG_LONG_MAX;
        durationMap[flight.from] = LONG_LONG_MAX;
        durationMap[flight.to] = LONG_LONG_MAX;
    }

    dist[start] = 0;
    durationMap[start] = 0;
    pq.insert({0, start});
    prev[start] = {};

    while (!pq.empty()) {
        auto [cost, current] = *pq.begin();
        pq.erase(pq.begin());

        if (current == end) break;

        for (const auto& [neighbor, fares] : flightGraph[current]) {
            double fare = fares.first;
            double duration = fares.second;
            double newCost=0;
            if(minimizeFare){
                newCost=cost + fare;
            }
            else{
                newCost=cost + duration;
            }

            if (minimizeFare && newCost < dist[neighbor]) {
                pq.erase({dist[neighbor], neighbor});
                dist[neighbor] = newCost;
                prev[neighbor] = prev[current];
                prev[neighbor].push_back(current); 
                durationMap[neighbor] = durationMap[current] + duration;
                pq.insert({newCost, neighbor});
            } else if (!minimizeFare && newCost < durationMap[neighbor]) {
                pq.erase({durationMap[neighbor], neighbor});
                durationMap[neighbor] = newCost;
                prev[neighbor] = prev[current];
                prev[neighbor].push_back(current);
                dist[neighbor] = dist[current] + fare;
                pq.insert({newCost, neighbor});
            }
        }
    }
    if (dist[end] == LONG_LONG_MAX) {
        cout << "No route found from " << start << " to " << end << "." << endl;
    } 
    else {
        cout << "Optimal route found with fare: $" << dist[end] 
             << " and duration: " << durationMap[end] << " hours." << endl;
        cout << "Path: ";
        for (const auto& step : prev[end]) {
            cout << step << " -> ";
        }
        cout << end << endl;
    }
}

// Show all bookings
void showBookings() {
    cout << "Current Bookings:\n--------------------------\n";
    for (const auto& booking : bookings) {
        cout << "Flight Number: " << booking.flightNumber 
             << ", Name: " << booking.name 
             << ", Contact: " << booking.contact 
             << ", Fare: $" << booking.fare 
             << ", PNR: " << booking.pnr << endl;
    }
}

// Update flight seat count
//work in progress
void updateFlightCount(const string& flightNumber, int change) {
    for (auto& flight : flights) {
        if (flight.flightNumber == flightNumber) {
            // Update count based on booking or cancellation
            flight.count += change; 
            return;
        }
    }
}

// Cancel a booking
void cancelBooking() {
    string pnr;
    cout << "Enter PNR of the booking you want to cancel: ";
    cin >> pnr;

    auto it = find_if(bookings.begin(), bookings.end(), [&](const Booking& booking) { 
        return booking.pnr == pnr; 
    });

    if (it != bookings.end()) {
        // Update flight seat count
        // updateFlightCount(it->flightNumber, 1); // Increment available seats
        bookings.erase(it); // Remove booking from vector
        cout << "Booking with PNR " << pnr << " has been cancelled." << endl;
    } else {
        cout << "Booking not found with PNR " << pnr << endl;
    }

    saveBookingsToFile(); // Save updated bookings to file
}

// Save updated bookings to file
void saveBookingsToFile() {
    ofstream file("booking.txt");
    if (file) {
        for (const auto& booking : bookings) {
            file << booking.flightNumber << " " << booking.name << " " << booking.contact 
                 << " " << booking.fare << " " << booking.pnr << endl;
        }
        file.close();
    } else {
        cout << "Could not open booking file" << endl;
    }
}

// Display main menu
void mainMenu() {
    while (true) {
        cout << "\nFlight Reservation System\n--------------------------\n";
        cout << "1. Search Flights\n";
        cout << "2. Book Flight\n";
        cout << "3. Show Bookings\n";
        cout << "4. Cancel Booking\n";
        cout << "5. Optimize Route\n";
        cout << "6. Exit\n";
        cout << "Enter your choice: ";
        
        int choice;
        cin >> choice;

        switch (choice) {
            case 1: {
                string from, to, date;
                cout << "From: ";
                cin >> from;
                cout << "To: ";
                cin >> to;
                cout << "Date (YYYY-MM-DD): ";
                cin >> date;
                searchFlights(from, to, date);
                break;
            }
            case 2: {
                string flightNumber, name, contact;
                cout << "Enter Flight Number: ";
                cin >> flightNumber;
                cout << "Enter Name: ";
                cin >> name;
                cout << "Enter Contact: ";
                cin >> contact;
                bookFlight(flightNumber, name, contact);
                break;
            }
            case 3:
                showBookings();
                break;
            case 4:
                cancelBooking();
                break;
            case 5: {
                string from, to, date;
                cout << "From: ";
                cin >> from;
                cout << "To: ";
                cin >> to;
                cout << "Date (YYYY-MM-DD): ";
                cin >> date;
                cout << "For 1.Fare, 0.Duration : ";
                bool minimizeFare;
                cin >> minimizeFare;
                optimizeRoute(from, to, date, minimizeFare);
                break;
            }
            case 6:
                cout << "Exiting." << endl;
                return;
            default:
                cout << "Invalid choice" << endl;
        }
    }
}
