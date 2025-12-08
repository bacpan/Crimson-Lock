#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtCore/QFile>
#include <iostream>
#include <exception>
#ifdef __linux__
#include <sys/resource.h>
#endif

#include "ui/MainWindow.h"
#include "core/PasswordGenerator.h"

/**
 * @brief Initialize application security settings
 */
void initializeSecurity() {
    // Disable core dumps for security
#ifdef __linux__
    struct rlimit rl;
    rl.rlim_cur = rl.rlim_max = 0;
    setrlimit(RLIMIT_CORE, &rl);
#endif

    // Set secure permissions on data directory
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
#ifdef __linux__
    // Set restrictive permissions (owner read/write/execute only)
    QFile::setPermissions(dataDir, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
#elif defined(_WIN32)
    // On Windows, the directory permissions are handled by the system
    // Additional security measures could be implemented here if needed
#endif
}

/**
 * @brief Check system requirements
 */
bool checkSystemRequirements() {
    // Check if hardware RNG is available
    if (!crimson::core::PasswordGenerator::isHardwareRngAvailable()) {
        QMessageBox::critical(nullptr, 
            "System Requirements", 
            "Hardware random number generator is not available.\n"
            "This application requires a secure entropy source for safe operation.\n\n"
            "Please ensure your system supports hardware randomness.");
        return false;
    }
    
    return true;
}

int main(int argc, char *argv[]) {
    try {
        // Create Qt application
        QApplication app(argc, argv);
        
        // Set application properties
        app.setApplicationName("Crimson Lock");
        app.setApplicationVersion("1.0.0");
        app.setOrganizationName("bacpan");
        app.setOrganizationDomain("crimsonsec.local");
        
        // Initialize security settings
        initializeSecurity();
        
        // Check system requirements
        if (!checkSystemRequirements()) {
            return 1;
        }
        
        // Show splash screen with security notice
        QString platformInfo;
#ifdef _WIN32
        platformInfo = "Windows";
#elif __linux__
        platformInfo = "Linux";
#elif __APPLE__
        platformInfo = "macOS";
#else
        platformInfo = "Unknown Platform";
#endif
        
        QMessageBox::information(nullptr,
            "Crimson Lock",
            QString("Crimson Lock v1.0.0\n"
                   "Running on: %1\n\n"
                   "Your credentials are safe and secure.\n").arg(platformInfo));
        
        // Create and show main window
        crimson::ui::MainWindow window;
        window.show();
        
        // Run application
        return app.exec();
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        QMessageBox::critical(nullptr, 
            "Fatal Error", 
            QString("A critical error occurred:\n\n%1\n\nThe application will now exit.").arg(e.what()));
        return 1;
    } catch (...) {
        std::cerr << "Unknown fatal error occurred" << std::endl;
        QMessageBox::critical(nullptr, 
            "Fatal Error", 
            "An unknown critical error occurred.\nThe application will now exit.");
        return 1;
    }
}
