#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <tuple>

using namespace std;

// Function to load the ratings matrix from a CSV file
vector<vector<int>> loadRatingsMatrix(const string &filename) {
    vector<vector<int>> ratings;
    ifstream file(filename);
    string line;
    bool isFirstLine = true;
    while (getline(file, line)) {
        if (isFirstLine) { // Skip the header row
            isFirstLine = false;
            continue;
        }
        stringstream ss(line);
        string value;
        vector<int> row;
        getline(ss, value, ','); // Skips the UserID column
        while (getline(ss, value, ',')) {
            row.push_back(stoi(value));
        }
        ratings.push_back(row);
    }
    file.close();

    // Debug output to ensure the matrix is loaded
    cout << "Loaded Ratings Matrix:" << endl;
    for (const auto &row : ratings) {
        for (int val : row) {
            cout << val << " ";
        }
        cout << endl;
    }

    return ratings;
}

// Function to calculate cosine similarity between two users
double calculateSimilarity(const vector<int> &user1, const vector<int> &user2) {
    double dotProduct = 0.0, normUser1 = 0.0, normUser2 = 0.0;
    for (size_t i = 0; i < user1.size(); i++) {
        dotProduct += user1[i] * user2[i];
        normUser1 += user1[i] * user1[i];
        normUser2 += user2[i] * user2[i];
    }
    if (normUser1 == 0 || normUser2 == 0) return 0; // Avoid division by zero
    return dotProduct / (sqrt(normUser1) * sqrt(normUser2));
}

// Function to predict ratings for a given user
vector<pair<int, double>> predictRatings(const vector<vector<int>> &ratings, int userIndex, int topN) {
    vector<pair<int, double>> recommendations;
    vector<double> similarities(ratings.size(), 0.0);

    // Calculate similarity of the target user with every other user
    for (size_t i = 0; i < ratings.size(); i++) {
        if (i != userIndex) {
            similarities[i] = calculateSimilarity(ratings[userIndex], ratings[i]);
        }
    }

    // Debug: Print user similarities
    cout << "User similarities for User " << userIndex + 1 << ":" << endl;
    for (size_t i = 0; i < similarities.size(); i++) {
        if (i != userIndex) {
            cout << "Similarity with User " << i + 1 << ": " << similarities[i] << endl;
        }
    }

    // Predict ratings for each movie not yet rated by the target user
    for (size_t movie = 0; movie < ratings[userIndex].size(); movie++) {
        if (ratings[userIndex][movie] == 0) { // If the movie is not rated
            double weightedSum = 0.0, similaritySum = 0.0;
            for (size_t otherUser = 0; otherUser < ratings.size(); otherUser++) {
                if (otherUser != userIndex && ratings[otherUser][movie] > 0) {
                    weightedSum += similarities[otherUser] * ratings[otherUser][movie];
                    similaritySum += fabs(similarities[otherUser]);
                }
            }
            if (similaritySum > 0) {
                double predictedRating = weightedSum / similaritySum;
                recommendations.emplace_back(movie, predictedRating);
            }
        }
    }

    // Sort recommendations by predicted rating in descending order
    sort(recommendations.begin(), recommendations.end(), [](const pair<int, double> &a, const pair<int, double> &b) {
        return b.second > a.second;
    });

    // Keep only the top N recommendations
    if (recommendations.size() > static_cast<size_t>(topN)) {
        recommendations.resize(topN);
    }

    return recommendations;
}

// Main function
int main() {
    string filename = "ratings.csv";
    vector<vector<int>> ratings = loadRatingsMatrix(filename);

    int userIndex = 0; // User for whom recommendations are generated (0-indexed)
    int topN = 3;      // Number of top recommendations

    vector<pair<int, double>> recommendations = predictRatings(ratings, userIndex, topN);

    if (recommendations.empty()) {
        cout << "No recommendations available for User " << userIndex + 1 << endl;
    } else {
        cout << "Top " << topN << " movie recommendations for User " << userIndex + 1 << ":" << endl;
        for (const auto &rec : recommendations) {
            cout << "Movie " << rec.first + 1 << " with predicted rating " << rec.second << endl;
        }
    }

    return 0;
}