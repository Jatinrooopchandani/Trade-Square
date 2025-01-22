#include <iostream>
#include <vector>
#include <SDL.h>
#include <SDL_ttf.h>
#include <chrono>
#include <thread>
#include<algorithm>
#include "./Color.hpp"
#include<string>
#include<conio.h>
#include<windows.h>
#include<fstream>
using namespace std;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int DELAY_MS = 500; // Delay in milliseconds between highlighting each Store

const SDL_Color RED = { 255, 0, 0, 255 };
const SDL_Color GREEN = { 0, 255, 0, 255 };
const SDL_Color BLUE = { 0, 0, 255, 255 };
const SDL_Color BLACK = { 0, 0, 0, 255 };

// Structure to represent a Store
struct Store {
    int x;
    int y;
    bool highlighted;
    int profit;
    int deadline;
    string name;
    Store(int _x, int _y, int _profit, int _deadline, string _name) : x(_x), y(_y), profit(_profit),
    deadline(_deadline), name(_name) {}
};

// Function to add a store to the file
void addStore(const Store& store) {

    ofstream outFile("stores.txt", ios::app);
    if (!outFile.is_open()) {
        cerr << "Error: Unable to open file for writing." << endl;
        return;
    }
    outFile << store.name << "," << store.x << "," << store.y << endl;
    outFile.close();
    cout << "Store added successfully." << endl;
}

// Function to delete a store from the file
void deleteStore(const string& storeName) {
    ifstream inFile("stores.txt");
    ofstream tempFile("temp.txt");
    if (!inFile || !tempFile) {
        cerr << "Error: Unable to open file for reading/writing." << endl;
        return;
    }

    string line;
    bool found = false;

    while (getline(inFile, line)) {

        // Extract store name from the line
        size_t pos = line.find(',');
        if (pos != string::npos) {
            string name = line.substr(0, pos);
            if (name != storeName) {
                tempFile << line << endl;
            }
            else {
                found = true;
            }
        }
    }
    inFile.close();
    tempFile.close();
    if (found) {
        remove("stores.txt");
        rename("temp.txt", "stores.txt");
        cout << "Store deleted successfully." << endl;
    }
    else {
        remove("temp.txt");
        cerr << "Store not found." << endl;
    }
}
// Function to fetch all store data from the file
vector<Store> fetchStores() {
    vector<Store> stores;
    ifstream inFile("stores.txt");
    if (!inFile) {
        cerr << "Error: Unable to open file for reading." << endl;
        return stores;
    }
    string line;
    while (getline(inFile, line)) {
        // Parse store information from the line
        size_t pos1 = line.find(',');
        size_t pos2 = line.find(',', pos1 + 1);
        if (pos1 != string::npos && pos2 != string::npos) {
            string name = line.substr(0, pos1);
            int x = stoi(line.substr(pos1 + 1, pos2 - pos1 - 1));
            int y = stoi(line.substr(pos2 + 1));
            stores.push_back({ x,y,0,0,name });
        }
    }
    inFile.close();
    return stores;
}

// Function to draw a filled circle
void drawFilledCircle(SDL_Renderer* renderer, int x, int y, int radius, const SDL_Color& color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int w = 0; w < radius * 2; ++w) {
        for (int h = 0; h < radius * 2; ++h) {
            int dx = radius - w;
            int dy = radius - h;
            if ((dx * dx + dy * dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer, x + dx, y + dy);
            }
        }
    }
}


// Function to draw text on the screen
void drawText(SDL_Renderer* renderer, int x, int y, const string& text, TTF_Font* font, const SDL_Color& color) {
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    int width, height;
    SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);
    SDL_Rect dstRect = { x - width / 2, y - height / 2, width, height };
    SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}


// Function to draw the graph of Stores
void drawStores(SDL_Renderer* renderer, const vector<Store>& Stores, const SDL_Color& color, TTF_Font* font) {
    for (const auto& Store : Stores) {
        drawFilledCircle(renderer, Store.x, Store.y, 5, color);
        drawText(renderer, Store.x, Store.y - 15, Store.name, font, BLACK); // Display store name above the
        Store
    }
}


// Function to calculate distance between two Stores
double distance(const Store& p1, const Store& p2) {
    return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
}


// Function to find K closest Stores to a given Store
vector<Store> kClosestStores(const Store& seller, const vector<Store>& Stores, int k) {
    vector<Store> closestStores;
    vector<double> distances;


    for (const auto& Store : Stores) {
        distances.push_back(distance(seller, Store));
    }


    // Sort distances and select K closest Stores
    vector<size_t> indices(distances.size());
        for (size_t i = 0; i < indices.size(); ++i) {
        indices[i] = i;
    }
    sort(indices.begin(), indices.end(), [&distances](size_t i, size_t j) {return distances[i] < distances[j];});
    for (int i = 0; i < k; ++i) {
        closestStores.push_back(Stores[indices[i]]);
    }
    return closestStores;
}



// Comparator function for sorting jobs
bool comparison(Store a, Store b)
{
    return (a.profit > b.profit);
}


// Function to schedule jobs based on deadlines and profit
vector<Store> scheduleJobs(const vector<Store>& Stores) {
    vector<Store> scheduledStores = Stores;

    sort(scheduledStores.begin(), scheduledStores.end(), comparison);
    vector<int> result(scheduledStores.size()); // To store result (Sequence of jobs)
    vector<bool> slot(scheduledStores.size()); // To keep track of free time slots


    // Initialize all slots to be free
    for (size_t i = 0; i < scheduledStores.size(); i++)
        slot[i] = false;


    // Iterate through all given jobs
    for (size_t i = 0; i < scheduledStores.size(); i++) {
        // Find a free slot for this job
        for (int j = min((int)scheduledStores.size(), scheduledStores[i].deadline) - 1; j >= 0; j--) {
            // Free slot found
            if (slot[j] == false) {
                result[j] = i; // Add this job to result
                slot[j] = true; // Make this slot occupied
                break;
            }
        }
    }



    // Arrange the jobs according to the scheduling
    vector<Store> scheduledJobs;
    for (size_t i = 0; i < scheduledStores.size(); i++)
    scheduledJobs.push_back(scheduledStores[result[i]]);
    return scheduledJobs;
}


int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        cerr << "SDL initialization failed: " << SDL_GetError() << endl;
        return 1;
    }
    // Initialize SDL TTF
    if (TTF_Init() == -1) {
        cerr << "SDL TTF initialization failed: " << TTF_GetError() << endl;
        SDL_Quit();
        return 1;
    }



    TTF_Font* font = TTF_OpenFont("arial.ttf", 24); // Change "arial.ttf" to the path of your font file
    SDL_Window* window = SDL_CreateWindow("Potential Buyers Graph",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH,
    SCREEN_HEIGHT, SDL_WINDOW_SHOWN);



    if (!window) {
        cerr << "Failed to create window: " << SDL_GetError() << endl;
        TTF_Quit(); // Cleanup TTF
        SDL_Quit();
        return 1;
    }


    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        cerr << "Failed to create renderer: " << SDL_GetError() << endl;
        SDL_DestroyWindow(window);
        TTF_Quit(); // Cleanup TTF
        SDL_Quit();
        return 1;
    }



    // Load city map image
    SDL_Surface* mapSurface = SDL_LoadBMP("delhi-outline-map.bmp");
    if (!mapSurface) {
        cerr << "Failed to load image: " << SDL_GetError() << endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit(); // Cleanup TTF
        SDL_Quit();
        return 1;
    }



    SDL_Texture* mapTexture = SDL_CreateTextureFromSurface(renderer, mapSurface);
    SDL_FreeSurface(mapSurface);
    int choice;
    string name;
    int x, y;
    char key;
    while (true) {
        menu:

        system("cls");
        cout << dye::yellow("1. Add a store") << endl;
        cout << dye::yellow("2. Delete a store") << endl;
        cout << dye::yellow("3. Fetch all store data") << endl;
        cout << dye::yellow("4. Look for the stores to sell to") << endl;
        cout << dye::yellow("5. Exit")<<endl;
        cout << dye::green("Enter your choice: ");
        cin >> choice;


        if (choice < 1 || choice>5)
        {
            cout << "Invalid choice";
            system("cls");
            goto menu;
        }

        switch (choice) {
            case 1:
                system("cls");
                cout << "Enter store name: ";
                cin.ignore();
                getline(cin, name);
                cout << "Enter x coordinate: ";
                cin >> x;
                cout << "Enter y coordinate: ";
                cin >> y;
                addStore({ x,y,0,0,name });
                key = _getch();
                goto menu;
            case 2:
                system("cls");
                cout << "Enter store name to delete: ";
                cin.ignore();
                getline(cin, name);
                deleteStore(name);
                cout << "Store deleted!" << endl;
                key = _getch();
                goto menu;
            case 3:
                {
                system("cls");
                Store seller(200, 200, 0, 0, "Seller");
                vector<Store> allStores = fetchStores();
                cout << "Store Data:" << endl;
                for (const auto& store : allStores) {
                cout << "Name: " << store.name << ", X: " << store.x << ", Y: " << store.y << endl;
                }
                // Render all stores on the map
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Set background color to white

                SDL_RenderClear(renderer); // Clear the screen

                // Render city map image
                SDL_RenderCopy(renderer, mapTexture, NULL, NULL);

                // Draw seller location
                drawFilledCircle(renderer, seller.x, seller.y, 5, RED);
                drawText(renderer, seller.x, seller.y - 15, "Seller", font, BLACK);


                // Draw all store locations in green
                drawStores(renderer, allStores, GREEN, font);

                // Present the rendered image
                SDL_RenderPresent(renderer);
                key = _getch();
                goto menu;
                }
                key = _getch();
                goto menu;
            case 4:
                system("cls");
                {
                    Store seller(200, 200, 0, 0, "Seller");
                    vector<Store> locationsToSellTo = fetchStores();


                    int k;
                    cout << "Enter the number of stores you want to sell to: ";
                    cin >> k;



                    vector<Store> nearestStores = kClosestStores(seller, locationsToSellTo, k);
                    int prof, dead;


                    for (int i = 0; i < k; ++i) {
                        cout << "Enter the profit and deadline of " << nearestStores[i].name << ": ";
                        cin >> prof >> dead;
                        nearestStores[i].profit = prof;
                        nearestStores[i].deadline = dead;
                    }
                    for (int i = 0; i < k; ++i) {
                        nearestStores[i].highlighted = true;
                    }


                    vector<Store> scheduledStores = scheduleJobs(nearestStores);
                    cout << "Job Scheduling Order:" << endl;
                    for (const auto& Store : scheduledStores) {
                        cout << Store.name << " -> ";
                    }

                    cout << endl;

                    // Event loop
                    bool quit = false;
                    SDL_Event e;
                    int totalProfit = 0;
                    int tpc = false;
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    SDL_RenderClear(renderer);
                    while (!quit) {
                        // Handle events
                        while (SDL_PollEvent(&e) != 0) {
                            if (e.type == SDL_QUIT) {
                            quit = true;
                            }
                        }


                        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                        SDL_RenderClear(renderer);


                        // Render city map image
                        SDL_RenderCopy(renderer, mapTexture, NULL, NULL);


                        // Draw seller location
                        drawFilledCircle(renderer, seller.x, seller.y, 5, RED);
                        drawText(renderer, seller.x, seller.y - 15, "Seller", font, BLACK);


                        // Draw locations to sell to (initially all green)
                        drawStores(renderer, locationsToSellTo, GREEN, font);
                        this_thread::sleep_for(chrono::milliseconds(DELAY_MS));


                        // Slowly highlight scheduled Stores with blue
                        const int DELAY_MS = 1500;


                        // Render all highlighted Stores in blue with a delay
                        for (auto& Store : scheduledStores) {
                            if (Store.highlighted) {
                                drawFilledCircle(renderer, Store.x, Store.y, 5, BLUE);
                                SDL_RenderPresent(renderer);
                                this_thread::sleep_for(chrono::milliseconds(DELAY_MS));
                                cout << "Profit earned from buyer at" << Store.name << "(" << Store.x << ", " << Store.y
                                << "): $" << Store.profit << endl;
                                totalProfit += Store.profit;
                                Store.highlighted = false; // Mark the Store as no longer needing to be highlighted
                            }
                        }

                        if (!tpc)
                        {
                        cout << "Total Profit: $" << totalProfit << endl;
                        tpc = true;
                        }
                    }
                    // Clean up and exit
                    SDL_DestroyTexture(mapTexture);
                    SDL_DestroyRenderer(renderer);
                    SDL_DestroyWindow(window);
                    TTF_CloseFont(font); // Close the font
                    TTF_Quit(); // Cleanup TTF
                    SDL_Quit();
                    key = _getch();
                    goto menu;
                }
            case 5:
                cout << "Exiting program." << endl;
                return 0;
                default:
                cout << "Invalid choice. Please try again." << endl;
        }
    }
    return 0;
}
