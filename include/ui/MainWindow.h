#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtCore/QTimer>
#include <memory>

namespace crimson {
namespace core {
class SecureVault;
}
}

namespace crimson {
namespace ui {

/**
 * @brief Main application window
 * 
 * Provides the primary interface for vault operations:
 * - Create new vault
 * - Open existing vault
 * - Manage vault entries
 * - Security features (auto-lock, etc.)
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onCreateVault();
    void onOpenVault();
    void onCreateEntry();
    void onViewVault();
    void onLockVault();
    void onSettings();
    void onAbout();
    void checkAutoLock();

private:
    // Core components
    std::unique_ptr<crimson::core::SecureVault> vault_;
    QTimer* auto_lock_timer_;
    
    // UI components
    QWidget* central_widget_;
    QVBoxLayout* main_layout_;
    
    // Welcome screen
    QWidget* welcome_widget_;
    QVBoxLayout* welcome_layout_;
    QLabel* title_label_;
    QLabel* subtitle_label_;
    QPushButton* create_vault_btn_;
    QPushButton* open_vault_btn_;
    
    // Vault screen
    QWidget* vault_widget_;
    QVBoxLayout* vault_layout_;
    QLabel* vault_status_label_;
    QHBoxLayout* vault_actions_layout_;
    QPushButton* create_entry_btn_;
    QPushButton* view_vault_btn_;
    QPushButton* lock_vault_btn_;
    
    // Status and info
    QLabel* security_status_label_;
    
    /**
     * @brief Initialize UI components
     */
    void setupUI();
    
    /**
     * @brief Setup menu bar
     */
    void setupMenuBar();
    
    /**
     * @brief Setup status bar
     */
    void setupStatusBar();
    
    /**
     * @brief Setup welcome screen UI
     */
    void setupWelcomeScreen();
    
    /**
     * @brief Setup vault screen UI
     */
    void setupVaultScreen();
    
    /**
     * @brief Switch to welcome screen
     */
    void showWelcomeScreen();
    
    /**
     * @brief Switch to vault screen
     */
    void showVaultScreen();
    
    /**
     * @brief Update security status display
     */
    void updateSecurityStatus();
    
    /**
     * @brief Show critical error message
     */
    void showCriticalError(const QString& title, const QString& message);
    
    /**
     * @brief Show information message
     */
    void showInfo(const QString& title, const QString& message);
    
    /**
     * @brief Get secure password input from user
     */
    QString getSecurePasswordInput(const QString& title, const QString& prompt);
};

} // namespace ui
} // namespace crimson
