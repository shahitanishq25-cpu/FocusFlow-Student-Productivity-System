#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <limits>

// ==========================================
// 1. Task Entity & Priority Enumeration
// ==========================================
enum class Priority { HIGH = 1, MEDIUM = 2, LOW = 3 };

class Task {
private:
    int id;
    std::string name;
    std::string subject;
    std::string deadline; // Format: YYYY-MM-DD
    Priority priority;
    int estimatedMinutes;
    int loggedMinutes;
    bool completed;

public:
    Task(int id, const std::string& name, const std::string& subject,
         const std::string& deadline, Priority priority,
         int estimatedMinutes, int loggedMinutes = 0, bool completed = false)
        : id(id), name(name), subject(subject), deadline(deadline),
          priority(priority), estimatedMinutes(estimatedMinutes),
          loggedMinutes(loggedMinutes), completed(completed) {}

    int getId() const { return id; }
    std::string getName() const { return name; }
    std::string getSubject() const { return subject; }
    std::string getDeadline() const { return deadline; }
    Priority getPriority() const { return priority; }
    int getEstimatedMinutes() const { return estimatedMinutes; }
    int getLoggedMinutes() const { return loggedMinutes; }
    bool isCompleted() const { return completed; }

    std::string getPriorityString() const {
        switch (priority) {
            case Priority::HIGH: return "HIGH";
            case Priority::MEDIUM: return "MEDIUM";
            case Priority::LOW:
            default: return "LOW";
        }
    }

    void markCompleted() { completed = true; }

    void addStudyMinutes(int minutes) {
        if (minutes > 0) loggedMinutes += minutes;
    }

    bool isOverdue(const std::string& currentDate) const {
        return !completed && (deadline < currentDate);
    }

    std::string serialize() const {
        std::ostringstream oss;
        oss << id << "|" << name << "|" << subject << "|" << deadline << "|"
            << static_cast<int>(priority) << "|" << estimatedMinutes << "|"
            << loggedMinutes << "|" << (completed ? 1 : 0);
        return oss.str();
    }

    static Task deserialize(const std::string& line) {
        std::istringstream iss(line);
        std::string token;
        
        std::getline(iss, token, '|'); int id = std::stoi(token);
        std::string name; std::getline(iss, name, '|');
        std::string subject; std::getline(iss, subject, '|');
        std::string deadline; std::getline(iss, deadline, '|');
        
        std::getline(iss, token, '|'); Priority priority = static_cast<Priority>(std::stoi(token));
        std::getline(iss, token, '|'); int est = std::stoi(token);
        std::getline(iss, token, '|'); int logged = std::stoi(token);
        std::getline(iss, token, '|'); int comp = std::stoi(token);

        return Task(id, name, subject, deadline, priority, est, logged, comp == 1);
    }
};

// ==========================================
// 2. Study Session Timer Entity
// ==========================================
class StudySession {
private:
    int sessionId;
    int taskId;
    std::string subject;
    int durationMinutes;
    std::string date;

public:
    StudySession(int sessionId, int taskId, const std::string& subject,
                 int durationMinutes, const std::string& date)
        : sessionId(sessionId), taskId(taskId), subject(subject),
          durationMinutes(durationMinutes), date(date) {}

    int getDurationMinutes() const { return durationMinutes; }
    std::string getSubject() const { return subject; }
    int getTaskId() const { return taskId; }

    static int runInteractiveTimer() {
        std::cout << "\n[Study Timer Started] Press ENTER when finished studying...";
        auto start = std::chrono::steady_clock::now();
        std::cin.get();
        auto end = std::chrono::steady_clock::now();
        
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
        int minutes = static_cast<int>(seconds / 60);
        if (minutes == 0 && seconds > 0) minutes = 1; // Minimum 1 minute credit for active sessions

        std::cout << "[Study Timer Stopped] Elapsed time: " << seconds 
                  << "s (" << minutes << " min(s) logged).\n";
        return minutes;
    }

    std::string serialize() const {
        std::ostringstream oss;
        oss << sessionId << "|" << taskId << "|" << subject << "|"
            << durationMinutes << "|" << date;
        return oss.str();
    }
};

// ==========================================
// 3. Task Manager Controller
// ==========================================
class TaskManager {
private:
    std::vector<Task> tasks;
    std::vector<StudySession> sessions;
    std::string tasksFile;
    std::string sessionsFile;

public:
    TaskManager(const std::string& tFile = "tasks.dat", const std::string& sFile = "sessions.dat")
        : tasksFile(tFile), sessionsFile(sFile) {
        loadData();
    }

    void loadData() {
        tasks.clear();
        sessions.clear();

        std::ifstream tFile(tasksFile);
        std::string line;
        while (std::getline(tFile, line)) {
            if (!line.empty()) {
                tasks.push_back(Task::deserialize(line));
            }
        }

        std::ifstream sFile(sessionsFile);
        while (std::getline(sFile, line)) {
            if (!line.empty()) {
                std::istringstream iss(line);
                std::string token;
                std::getline(iss, token, '|'); int sId = std::stoi(token);
                std::getline(iss, token, '|'); int tId = std::stoi(token);
                std::string subj; std::getline(iss, subj, '|');
                std::getline(iss, token, '|'); int dur = std::stoi(token);
                std::string date; std::getline(iss, date, '|');
                sessions.push_back(StudySession(sId, tId, subj, dur, date));
            }
        }
    }

    void saveData() const {
        std::ofstream tFile(tasksFile);
        for (const auto& t : tasks) {
            tFile << t.serialize() << "\n";
        }

        std::ofstream sFile(sessionsFile);
        for (const auto& s : sessions) {
            sFile << s.serialize() << "\n";
        }
    }

    void addTask(const std::string& name, const std::string& subject,
                 const std::string& deadline, Priority priority, int estimatedMinutes) {
        int newId = tasks.empty() ? 1 : tasks.back().getId() + 1;
        tasks.push_back(Task(newId, name, subject, deadline, priority, estimatedMinutes));
        saveData();
    }

    const std::vector<Task>& getAllTasks() const {
        return tasks;
    }

    Task* findTaskById(int id) {
        for (auto& t : tasks) {
            if (t.getId() == id) return &t;
        }
        return nullptr;
    }

    void markTaskCompleted(int id) {
        Task* t = findTaskById(id);
        if (t) {
            t->markCompleted();
            saveData();
            std::cout << "[Success] Task ID " << id << " marked as COMPLETED.\n";
        } else {
            std::cout << "[Error] Task ID " << id << " not found.\n";
        }
    }

    void deleteTask(int id) {
        auto it = std::remove_if(tasks.begin(), tasks.end(), [id](const Task& t) {
            return t.getId() == id;
        });
        if (it != tasks.end()) {
            tasks.erase(it, tasks.end());
            saveData();
            std::cout << "[Success] Task ID " << id << " deleted.\n";
        } else {
            std::cout << "[Warning] Task ID " << id << " not found for deletion.\n";
        }
    }

    void searchTasks(const std::string& keyword) const {
        std::string lowerKey = keyword;
        std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);
        
        std::cout << "\n--- Search Results for \\"" << keyword << "\\" ---\n";
        bool found = false;
        for (const auto& t : tasks) {
            std::string name = t.getName();
            std::string subj = t.getSubject();
            std::transform(name.begin(), name.end(), name.begin(), ::tolower);
            std::transform(subj.begin(), subj.end(), subj.begin(), ::tolower);

            if (name.find(lowerKey) != std::string::npos || subj.find(lowerKey) != std::string::npos) {
                std::cout << "ID: " << t.getId() << " | " << t.getName() 
                          << " | Subject: " << t.getSubject() << " | Status: " 
                          << (t.isCompleted() ? "COMPLETED" : "PENDING") << "\n";
                found = true;
            }
        }
        if (!found) std::cout << "No matching tasks found.\n";
    }

    void addStudySession(int taskId, int duration, const std::string& date) {
        int newId = sessions.empty() ? 1 : sessions.back().getTaskId() + 1;
        Task* t = findTaskById(taskId);
        std::string subj = t ? t->getSubject() : "General";
        
        if (t) {
            t->addStudyMinutes(duration);
        }
        sessions.push_back(StudySession(newId, taskId, subj, duration, date));
        saveData();
    }

    int getTotalStudyMinutes() const {
        int total = 0;
        for (const auto& s : sessions) {
            total += s.getDurationMinutes();
        }
        return total;
    }
};

// ==========================================
// 4. Productivity Dashboard View
// ==========================================
class Dashboard {
private:
    const TaskManager& taskManager;

public:
    Dashboard(const TaskManager& tm) : taskManager(tm) {}

    void displaySummary(const std::string& today) const {
        const auto& tasks = taskManager.getAllTasks();
        int total = static_cast<int>(tasks.size());
        int completed = 0, overdue = 0;
        int totalMins = taskManager.getTotalStudyMinutes();

        for (const auto& t : tasks) {
            if (t.isCompleted()) {
                completed++;
            } else if (t.isOverdue(today)) {
                overdue++;
            }
        }

        int pending = total - completed;
        double rate = (total > 0) ? (100.0 * completed / total) : 0.0;

        std::cout << "\n============================================\n"
                  << "         FOCUSFLOW DASHBOARD SUMMARY        \n"
                  << "============================================\n"
                  << "Total Tasks Created : " << total << "\n"
                  << "Completed Tasks     : " << completed << " (" << std::fixed << std::setprecision(1) << rate << "%)\n"
                  << "Pending Tasks       : " << pending << " (Overdue: " << overdue << ")\n"
                  << "Total Study Logged  : " << (totalMins / 60) << " hrs " << (totalMins % 60) << " mins (" << totalMins << " mins)\n"
                  << "============================================\n";
    }
};

// ==========================================
// 5. Main Execution Loop & Interface
// ==========================================
std::string getCurrentDate() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d");
    return oss.str();
}

void clearInputBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int main() {
    TaskManager manager;
    Dashboard dashboard(manager);

    int choice = 0;
    do {
        std::cout << "\n============================================\n"
                  << "     FOCUSFLOW - STUDENT PRODUCTIVITY       \n"
                  << "============================================\n"
                  << "1. Add New Task\n"
                  << "2. View Tasks (All / Pending / Completed / Overdue)\n"
                  << "3. Manage Task (Mark Complete / Delete / Search)\n"
                  << "4. Start Study Session\n"
                  << "5. View Productivity Dashboard\n"
                  << "6. Save & Exit\n"
                  << "--------------------------------------------\n"
                  << "Enter choice: ";

        if (!(std::cin >> choice)) {
            std::cout << "[Error] Invalid input. Please enter a number.\n";
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        switch (choice) {
            case 1: {
                std::string name, subject, deadline;
                int prioInt, estMins;
                
                std::cout << "Enter Task Name: ";
                std::getline(std::cin, name);
                std::cout << "Enter Subject Code: ";
                std::getline(std::cin, subject);
                std::cout << "Enter Deadline (YYYY-MM-DD): ";
                std::getline(std::cin, deadline);
                std::cout << "Enter Priority ([1] HIGH, [2] MEDIUM, [3] LOW): ";
                std::cin >> prioInt;
                std::cout << "Enter Estimated Minutes: ";
                std::cin >> estMins;
                clearInputBuffer();

                Priority p = Priority::MEDIUM;
                if (prioInt == 1) p = Priority::HIGH;
                else if (prioInt == 3) p = Priority::LOW;

                manager.addTask(name, subject, deadline, p, estMins);
                std::cout << "[Success] Task added successfully!\n";
                break;
            }
            case 2: {
                int filter;
                std::cout << "Filter: [1] All [2] Pending [3] Completed [4] Overdue -> Choice: ";
                std::cin >> filter;
                clearInputBuffer();

                std::string today = getCurrentDate();
                const auto& tasks = manager.getAllTasks();
                
                std::cout << "\nID | Task Name          | Subject | Deadline   | Priority | Est/Log  | Status\n"
                          << "--------------------------------------------------------------------------------\n";
                for (const auto& t : tasks) {
                    bool show = false;
                    bool overdue = t.isOverdue(today);
                    
                    if (filter == 1) show = true;
                    else if (filter == 2 && !t.isCompleted()) show = true;
                    else if (filter == 3 && t.isCompleted()) show = true;
                    else if (filter == 4 && overdue) show = true;

                    if (show) {
                        std::cout << t.getId() << "  | " 
                                  << std::left << std::setw(18) << t.getName().substr(0, 18) << " | "
                                  << std::setw(7) << t.getSubject() << " | "
                                  << t.getDeadline() << " | "
                                  << std::setw(8) << t.getPriorityString() << " | "
                                  << t.getEstimatedMinutes() << "/" << t.getLoggedMinutes() << "m | "
                                  << (t.isCompleted() ? "COMPLETED" : (overdue ? "OVERDUE" : "PENDING")) << "\n";
                    }
                }
                break;
            }
            case 3: {
                int subChoice;
                std::cout << "[1] Mark Complete [2] Delete Task [3] Search Task -> Choice: ";
                std::cin >> subChoice;
                clearInputBuffer();

                if (subChoice == 1) {
                    int id;
                    std::cout << "Enter Task ID to mark completed: ";
                    std::cin >> id;
                    clearInputBuffer();
                    manager.markTaskCompleted(id);
                } else if (subChoice == 2) {
                    int id;
                    std::cout << "Enter Task ID to delete: ";
                    std::cin >> id;
                    clearInputBuffer();
                    manager.deleteTask(id);
                } else if (subChoice == 3) {
                    std::string kw;
                    std::cout << "Enter keyword to search: ";
                    std::getline(std::cin, kw);
                    manager.searchTasks(kw);
                }
                break;
            }
            case 4: {
                int taskId;
                std::cout << "Enter Task ID to associate study session with: ";
                std::cin >> taskId;
                clearInputBuffer();

                int mins = StudySession::runInteractiveTimer();
                manager.addStudySession(taskId, mins, getCurrentDate());
                std::cout << "[Success] Study session recorded and credited to Task ID " << taskId << "!\n";
                break;
            }
            case 5: {
                dashboard.displaySummary(getCurrentDate());
                break;
            }
            case 6: {
                manager.saveData();
                std::cout << "Data saved successfully. Exiting FocusFlow. Goodbye!\n";
                break;
            }
            default:
                std::cout << "[Error] Invalid choice. Please select between 1 and 6.\n";
        }
    } while (choice != 6);

    return 0;
}
