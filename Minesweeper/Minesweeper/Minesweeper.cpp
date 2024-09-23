#include <iostream>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <map>
#include <fstream>
#include <string>

using namespace std;

struct User {
    string login;
    string password;
};


map<string, string> loadUsersFromFile(const string& filename) {
    map<string, string> users;
    ifstream file(filename);
    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            size_t pos = line.find(':');
            if (pos != string::npos) {
                string login = line.substr(0, pos);
                string password = line.substr(pos + 1);
                users[login] = password;
            }
        }
        file.close();
    }
    return users;
}

bool authorizeUser(const string& login, const string& password) {
    map<string, string> users = loadUsersFromFile("users.txt");
    if (users.find(login) != users.end()) {
        if (users[login] == password) {
            return true;
        }
    }

    return false;
}

void showFile(const string& filename) {
    ifstream file(filename);
    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            cout << line << endl;
        }
        file.close();
    }
    else {
        cout << "Error: Could not open help file." << endl;
    }
}


const int ROWS = 10; 
const int COLS = 10; 
const int MINES = 5;

struct Cell {
    bool isMine; 
    int adjacentMines;
    bool isRevealed; 
    bool isFlagged;
};

int countAdjacentMines(const vector<vector<Cell>>& field, int row, int col);

void generateField(vector<vector<Cell>>& field) {
    srand(time(0));

    int minesPlaced = 0;
    while (minesPlaced < MINES) {
        int row = rand() % ROWS;
        int col = rand() % COLS;

        if (!field[row][col].isMine) {
            field[row][col].isMine = true;
            minesPlaced++;
        }
    }

    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            if (!field[row][col].isMine) {
                field[row][col].adjacentMines = countAdjacentMines(field, row, col);
            }
        }
    }
}

int countAdjacentMines(const vector<vector<Cell>>& field, int row, int col) {
    int count = 0;
    for (int i = row - 1; i <= row + 1; i++) {
        for (int j = col - 1; j <= col + 1; j++) {
            if (i >= 0 && i < ROWS && j >= 0 && j < COLS && field[i][j].isMine) {
                count++;
            }
        }
    }
    return count;
}

void displayField(const vector<vector<Cell>>& field) {
    cout << "   ";
    for (int col = 0; col < COLS; col++) {
        cout << col + 1 << "  ";
    }
    cout << endl;

    for (int row = 0; row < ROWS; row++) {
        cout << row + 1 << "  ";
        for (int col = 0; col < COLS; col++) {
            if (field[row][col].isRevealed) {
                if (field[row][col].isMine) {
                    cout << "B  ";
                }
                else {
                    if (field[row][col].adjacentMines > 0) {
                        cout << field[row][col].adjacentMines << "  ";
                    }
                    else {
                        cout << "   ";
                    }
                }
            }
            else {
                if (field[row][col].isFlagged) {
                    cout << "F  ";
                }
                else {
                    cout << "*  ";
                }
            }
        }
        cout << endl;
    }
}

void openCell(vector<vector<Cell>>& field, int row, int col, int& remainingMines);

bool handleInput(vector<vector<Cell>>& field, int& remainingMines) {
    int row, col;
    char action;
    cout << " Input cell coordinates (Row, Col) action (o - open, f - flag): ";
    cin >> row >> col >> action;

    row--;
    col--;

    if (row >= 0 && row < ROWS && col >= 0 && col < COLS) {
        if (action == 'o') {
            if (field[row][col].isMine) {
                cout << "Yoy've lost. Clicked on bomb cell." << endl;
                displayField(field);
                return false;
            }
            else {
                openCell(field, row, col, remainingMines);
            }
        }
        else if (action == 'f') {
            if (!field[row][col].isRevealed) {
                field[row][col].isFlagged = !field[row][col].isFlagged;
                if (field[row][col].isFlagged) {
                    remainingMines--;
                }
                else {
                    remainingMines++;
                }
            }
        }
    }
    else {
        cout << "Wrong coordinates." << endl;
    }

    return true;
}

void openCell(vector<vector<Cell>>& field, int row, int col, int& remainingMines) {
    if (!field[row][col].isRevealed) {
        field[row][col].isRevealed = true;

        if (field[row][col].adjacentMines == 0) {

            for (int i = row - 1; i <= row + 1; i++) {
                for (int j = col - 1; j <= col + 1; j++) {
                    if (i >= 0 && i < ROWS && j >= 0 && j < COLS) {
                        openCell(field, i, j, remainingMines);
                    }
                }
            }
        }
    }
}

bool isVictory(const vector<vector<Cell>>& field, int remainingMines) {
    if (remainingMines == 0) {
        for (int row = 0; row < ROWS; row++) {
            for (int col = 0; col < COLS; col++) {
                if (!field[row][col].isMine && !field[row][col].isRevealed) {
                    return false;
                }
            }
        }
        return true;
    }
    return false;
}

void saveGame(const vector<vector<Cell>>& field, int remainingMines, const string& filename, const string& username) {
    ofstream file(filename);
    if (file.is_open()) {
        file << username << endl;
        file << ROWS << " " << COLS << " " << MINES << endl;
        for (int row = 0; row < ROWS; row++) {
            for (int col = 0; col < COLS; col++) {
                file << field[row][col].isMine << " " << field[row][col].isRevealed << " " << field[row][col].adjacentMines << " ";
            }
            file << endl;
        }
        file << remainingMines << endl;
        file.close();
    }
}

bool loadGame(vector<vector<Cell>>& field, int& remainingMines, const string& filename, const string& username) {
    ifstream file(filename);
    if (file.is_open()) {
        string savedUsername;
        getline(file, savedUsername);
        if (savedUsername != username) {
            cout << "This save file belongs to another user." << endl;
            file.close();
            return false;
        }
        int rows, cols, mines;
        file >> rows >> cols >> mines;
        if (rows != ROWS || cols != COLS || mines != MINES) {
            cout << "Invalid game data." << endl;
            file.close();
            return false;
        }

        field.resize(rows);
        for (int row = 0; row < rows; row++) {
            field[row].resize(cols);
            for (int col = 0; col < cols; col++) {
                file >> field[row][col].isMine >> field[row][col].isRevealed >> field[row][col].adjacentMines;
            }
        }

        file >> remainingMines;
        file.close();
        return true;
    }
    return false;
}

int main() {
    string login, password;

    cout << "Input login: ";
    cin >> login;

    cout << "Input password: ";
    cin >> password;

    if (authorizeUser(login, password)) {
        cout << "Authoritized successful! Wellcome!" << endl;
        vector<vector<Cell>> field(ROWS, vector<Cell>(COLS));
        int remainingMines = MINES;

        while (true) {
            cout << "1. Play." << endl;
            cout << "2. Load Game." << endl;
            cout << "3. Help." << endl;
            cout << "4. About." << endl;
            cout << "5. Exit." << endl;

            int choice;
            cin >> choice;

            switch (choice) {
            case 1: {
                generateField(field);
                bool gameOver = false;
                while (!gameOver) {
                    system("cls");
                    displayField(field);
                    cout << "Bombs left: " << remainingMines << endl;
                    if (!handleInput(field, remainingMines)) {
                        gameOver = true; 
                    }
                    if (isVictory(field, remainingMines)) {
                        cout << "You won!" << endl;
                        gameOver = true; 
                    }

                    if (!gameOver) {
                        cout << "Do you want to save the game? (y/n): ";
                        char saveChoice;
                        cin >> saveChoice;
                        if (saveChoice == 'y') {
                            string filename = "savegame_" + login + ".txt";
                            saveGame(field, remainingMines, filename, login);
                            cout << "Game saved successfully!" << endl;
                        }
                    }
                }
                break;
            }
            case 2: {
                string filename = "savegame_" + login + ".txt";
                if (loadGame(field, remainingMines, filename, login)) {
                    cout << "Game loaded successfully!" << endl;

                    bool gameOver = false;

                    while (!gameOver) {
                        system("cls");
                        displayField(field);
                        cout << "Bombs left: " << remainingMines << endl;

                        if (!handleInput(field, remainingMines)) {
                            gameOver = true;
                        }
                        if (isVictory(field, remainingMines)) {
                            cout << "You won!" << endl;
                            gameOver = true;
                        }

                        if (!gameOver) {
                            cout << "Do you want to save the game? (y/n): ";
                            char saveChoice;
                            cin >> saveChoice;
                            if (saveChoice == 'y') {
                                string filename = "savegame_" + login + ".txt";
                                saveGame(field, remainingMines, filename, login);
                                cout << "Game saved successfully!" << endl;
                            }
                        }
                    }
                }
                else {
                    cout << "Failed to load game." << endl;
                }
                break;
            }
            case 3: {
                system("cls");
                showFile("help.txt");
                break;
            }
            case 4: {
                system("cls");
                showFile("about.txt");
                break;
            }
            case 5: {
                cout << "Exiting game..." << endl;
                return 0;
            }
            default:
                cout << "Wrong input" << endl;
            }
        }
    }
    else {
        cout << "Wrong login or password." << endl;
    }

    return 0;
}