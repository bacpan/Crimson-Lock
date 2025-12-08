#pragma once

#include <QtWidgets/QDialog>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QCheckBox>
#include <QtCore/QTimer>
#include "../core/VaultEntry.h"

namespace crimson {
namespace ui {

/**
 * @brief Dialog for creating new vault entries
 * 
 * Allows users to:
 * - Enter a label for the entry
 * - View generated credentials once
 * - Save the entry to the vault
 */
class VaultCreationDialog : public QDialog {
    Q_OBJECT

public:
    explicit VaultCreationDialog(QWidget* parent = nullptr);
    ~VaultCreationDialog();
    
    /**
     * @brief Get the created vault entry
     * @return Created entry (empty if cancelled)
     */
    crimson::core::VaultEntry getEntry() const { return entry_; }

private slots:
    void onGenerateCredentials();
    void onSaveEntry();
    void onCancel();
    void onCopyUsername();
    void onCopyPassword();
    void onShowPasswordTemporary();
    void hidePasswordDisplay();

private:
    // Entry data
    crimson::core::VaultEntry entry_;
    bool credentials_generated_;
    bool entry_saved_;
    
    // UI components
    QVBoxLayout* main_layout_;
    QFormLayout* form_layout_;
    QHBoxLayout* button_layout_;
    
    // Input fields
    QLineEdit* label_input_;
    
    // Security features
    QTimer* password_hide_timer_;
    QLabel* security_warning_;
    
    // Generated credentials display
    QWidget* credentials_widget_;
    QVBoxLayout* credentials_layout_;
    QLabel* credentials_title_;
    QLineEdit* username_display_;
    QLineEdit* password_display_;
    QPushButton* copy_username_btn_;
    QPushButton* copy_password_btn_;
    QPushButton* show_password_btn_;
    
    // Action buttons
    QPushButton* generate_btn_;
    QPushButton* save_btn_;
    QPushButton* cancel_btn_;
    
    /**
     * @brief Initialize UI components
     */
    void setupUI();
    
    /**
     * @brief Setup credentials display area
     */
    void setupCredentialsDisplay();
    
    /**
     * @brief Show/hide credentials section
     */
    void setCredentialsVisible(bool visible);
    
    /**
     * @brief Update button states
     */
    void updateButtonStates();
    
    /**
     * @brief Clear sensitive display data
     */
    void clearSensitiveDisplay();
    
    /**
     * @brief Copy text to clipboard with auto-clear
     */
    void copyToClipboard(const QString& text, const QString& description);
    
    /**
     * @brief Validate input fields
     */
    bool validateInput();
};

} // namespace ui
} // namespace crimson
