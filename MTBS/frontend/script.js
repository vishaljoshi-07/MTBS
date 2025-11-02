// MTBS Frontend - Core Banking System Interface
class MTBSFrontend {
    constructor() {
        this.currentTheme = localStorage.getItem('theme') || 'light';
        this.accounts = [];
        this.transactions = [];
        this.init();
    }

    init() {
        this.setupEventListeners();
        this.setTheme(this.currentTheme);
        this.loadInitialData();
        this.startRealTimeUpdates();
    }

    setupEventListeners() {
        // Theme toggle
        document.getElementById('theme-toggle').addEventListener('click', () => this.toggleTheme());

        // Navigation
        document.querySelectorAll('.nav-item').forEach(item => {
            item.addEventListener('click', (e) => {
                e.preventDefault();
                this.showSection(item.dataset.section);
            });
        });

        // Forms
        document.getElementById('create-account-form').addEventListener('submit', (e) => {
            e.preventDefault();
            this.createAccount();
        });

        document.getElementById('deposit-form').addEventListener('submit', (e) => {
            e.preventDefault();
            this.processDeposit();
        });

        document.getElementById('withdraw-form').addEventListener('submit', (e) => {
            e.preventDefault();
            this.processWithdraw();
        });

        document.getElementById('transfer-form').addEventListener('submit', (e) => {
            e.preventDefault();
            this.processTransfer();
        });

        // Quick actions
        document.getElementById('generate-data-btn').addEventListener('click', () => this.generateSampleData());
        document.getElementById('clear-data-btn').addEventListener('click', () => this.clearAllData());

        // Modal close
        document.getElementById('modal-close').addEventListener('click', () => this.hideModal());
        document.getElementById('modal-overlay').addEventListener('click', (e) => {
            if (e.target === e.currentTarget) {
                this.hideModal();
            }
        });
    }

    setTheme(theme) {
        this.currentTheme = theme;
        document.documentElement.setAttribute('data-theme', theme);
        localStorage.setItem('theme', theme);
        
        const icon = document.querySelector('#theme-toggle i');
        icon.className = theme === 'dark' ? 'fas fa-sun' : 'fas fa-moon';
    }

    toggleTheme() {
        this.setTheme(this.currentTheme === 'light' ? 'dark' : 'light');
    }

    showSection(sectionId) {
        document.querySelectorAll('.content-section').forEach(s => s.classList.remove('active'));
        document.querySelectorAll('.nav-item').forEach(n => n.classList.remove('active'));
        
        document.getElementById(sectionId).classList.add('active');
        document.querySelector(`[data-section="${sectionId}"]`).classList.add('active');
        
        this.updateSectionContent(sectionId);
    }

    updateSectionContent(sectionId) {
        switch(sectionId) {
            case 'dashboard': this.updateDashboard(); break;
            case 'accounts': this.updateAccountsList(); break;
            case 'transactions': this.updateTransactionHistory(); break;
            case 'threads': this.updateThreadMonitor(); break;
            case 'performance': this.updatePerformanceMetrics(); break;
        }
    }

    createAccount() {
        const form = document.getElementById('create-account-form');
        const formData = new FormData(form);
        
        const holderName = formData.get('holderName');
        const initialBalance = parseFloat(formData.get('initialBalance')) || 0;

        if (!holderName.trim()) {
            this.showModal('Error', 'Account holder name is required');
            return;
        }

        const accountNumber = 'ACC' + Date.now().toString().slice(-6);
        const account = {
            accountNumber,
            holderName,
            balance: initialBalance,
            status: 'ACTIVE',
            createdAt: new Date(),
            transactions: []
        };

        this.accounts.push(account);

        if (initialBalance > 0) {
            const transaction = {
                id: 'TXN_' + Date.now(),
                type: 'DEPOSIT',
                amount: initialBalance,
                description: 'Initial deposit',
                status: 'SUCCESS',
                timestamp: new Date(),
                fromAccount: '',
                toAccount: accountNumber
            };
            account.transactions.push(transaction);
            this.transactions.push(transaction);
        }

        this.updateAccountsList();
        this.updateDashboard();
        this.showModal('Success', `Account created! Number: ${accountNumber}`);
        form.reset();
    }

    processDeposit() {
        const form = document.getElementById('deposit-form');
        const formData = new FormData(form);
        
        const accountNumber = formData.get('accountNumber');
        const amount = parseFloat(formData.get('amount'));
        const description = formData.get('description') || 'Deposit';

        if (!this.validateTransaction(accountNumber, amount)) return;

        const account = this.accounts.find(acc => acc.accountNumber === accountNumber);
        if (!account) {
            this.showModal('Error', 'Account not found');
            return;
        }

        account.balance += amount;
        
        const transaction = {
            id: 'TXN_' + Date.now(),
            type: 'DEPOSIT',
            amount: amount,
            description: description,
            status: 'SUCCESS',
            timestamp: new Date(),
            fromAccount: '',
            toAccount: accountNumber
        };

        account.transactions.push(transaction);
        this.transactions.push(transaction);

        this.updateAccountsList();
        this.updateTransactionHistory();
        this.updateDashboard();
        this.showModal('Success', `Deposit of $${amount.toFixed(2)} processed`);
        form.reset();
    }

    processWithdraw() {
        const form = document.getElementById('withdraw-form');
        const formData = new FormData(form);
        
        const accountNumber = formData.get('accountNumber');
        const amount = parseFloat(formData.get('amount'));
        const description = formData.get('description') || 'Withdrawal';

        if (!this.validateTransaction(accountNumber, amount)) return;

        const account = this.accounts.find(acc => acc.accountNumber === accountNumber);
        if (!account) {
            this.showModal('Error', 'Account not found');
            return;
        }

        if (account.balance < amount) {
            this.showModal('Error', 'Insufficient funds');
            return;
        }

        account.balance -= amount;
        
        const transaction = {
            id: 'TXN_' + Date.now(),
            type: 'WITHDRAW',
            amount: amount,
            description: description,
            status: 'SUCCESS',
            timestamp: new Date(),
            fromAccount: accountNumber,
            toAccount: ''
        };

        account.transactions.push(transaction);
        this.transactions.push(transaction);

        this.updateAccountsList();
        this.updateTransactionHistory();
        this.updateDashboard();
        this.showModal('Success', `Withdrawal of $${amount.toFixed(2)} processed`);
        form.reset();
    }

    processTransfer() {
        const form = document.getElementById('transfer-form');
        const formData = new FormData(form);
        
        const fromAccount = formData.get('fromAccount');
        const toAccount = formData.get('toAccount');
        const amount = parseFloat(formData.get('amount'));
        const description = formData.get('description') || 'Transfer';

        if (!this.validateTransaction(fromAccount, amount)) return;

        if (fromAccount === toAccount) {
            this.showModal('Error', 'Cannot transfer to same account');
            return;
        }

        const fromAcc = this.accounts.find(acc => acc.accountNumber === fromAccount);
        const toAcc = this.accounts.find(acc => acc.accountNumber === toAccount);

        if (!fromAcc || !toAcc) {
            this.showModal('Error', 'One or both accounts not found');
            return;
        }

        if (fromAcc.balance < amount) {
            this.showModal('Error', 'Insufficient funds');
            return;
        }

        fromAcc.balance -= amount;
        toAcc.balance += amount;
        
        const transactionId = 'TXN_' + Date.now();
        
        const outgoingTransaction = {
            id: transactionId + '_OUT',
            type: 'TRANSFER',
            amount: amount,
            description: description,
            status: 'SUCCESS',
            timestamp: new Date(),
            fromAccount: fromAccount,
            toAccount: toAccount
        };

        const incomingTransaction = {
            id: transactionId + '_IN',
            type: 'TRANSFER',
            amount: amount,
            description: description,
            status: 'SUCCESS',
            timestamp: new Date(),
            fromAccount: fromAccount,
            toAccount: toAccount
        };

        fromAcc.transactions.push(outgoingTransaction);
        toAcc.transactions.push(incomingTransaction);
        this.transactions.push(outgoingTransaction);

        this.updateAccountsList();
        this.updateTransactionHistory();
        this.updateDashboard();
        this.showModal('Success', `Transfer of $${amount.toFixed(2)} completed`);
        form.reset();
    }

    validateTransaction(accountNumber, amount) {
        if (!accountNumber) {
            this.showModal('Error', 'Please select an account');
            return false;
        }
        if (!amount || amount <= 0) {
            this.showModal('Error', 'Please enter a valid amount');
            return false;
        }
        return true;
    }

    updateDashboard() {
        document.getElementById('total-accounts').textContent = this.accounts.length;
        document.getElementById('total-transactions').textContent = this.transactions.length;
        document.getElementById('active-threads').textContent = Math.floor(Math.random() * 10) + 5;
        document.getElementById('queue-size').textContent = Math.floor(Math.random() * 20);
        this.updateActivityFeed();
    }

    updateActivityFeed() {
        const feedContainer = document.getElementById('activity-feed');
        const activities = [
            'Account created successfully',
            'Deposit processed',
            'Withdrawal completed',
            'Transfer executed',
            'System backup completed'
        ];

        const randomActivity = activities[Math.floor(Math.random() * activities.length)];
        const timestamp = new Date().toLocaleTimeString();

        const feedItem = document.createElement('div');
        feedItem.className = 'feed-item fade-in';
        feedItem.innerHTML = `
            <i class="fas fa-info-circle"></i>
            <span>${randomActivity}</span>
            <small class="timestamp">${timestamp}</small>
        `;

        feedContainer.insertBefore(feedItem, feedContainer.firstChild);

        while (feedContainer.children.length > 10) {
            feedContainer.removeChild(feedContainer.lastChild);
        }
    }

    updateAccountsList() {
        const accountsGrid = document.getElementById('accounts-grid');
        const accountSelects = [
            document.getElementById('deposit-account'),
            document.getElementById('withdraw-account'),
            document.getElementById('transfer-from'),
            document.getElementById('transfer-to')
        ];

        accountsGrid.innerHTML = '';
        accountSelects.forEach(select => {
            select.innerHTML = '<option value="">Select Account</option>';
        });

        this.accounts.forEach(account => {
            const accountCard = this.createAccountCard(account);
            accountsGrid.appendChild(accountCard);

            accountSelects.forEach(select => {
                const option = document.createElement('option');
                option.value = account.accountNumber;
                option.textContent = `${account.accountNumber} - ${account.holderName}`;
                select.appendChild(option);
            });
        });
    }

    createAccountCard(account) {
        const card = document.createElement('div');
        card.className = 'account-card fade-in';
        card.innerHTML = `
            <div class="account-header">
                <span class="account-number">${account.accountNumber}</span>
                <span class="account-status ${account.status.toLowerCase()}">${account.status}</span>
            </div>
            <div class="account-details">
                <p><strong>Holder:</strong> ${account.holderName}</p>
                <p><strong>Created:</strong> ${account.createdAt.toLocaleDateString()}</p>
                <p><strong>Transactions:</strong> ${account.transactions.length}</p>
            </div>
            <div class="account-balance">$${account.balance.toFixed(2)}</div>
        `;
        return card;
    }

    updateTransactionHistory() {
        const historyContainer = document.getElementById('transaction-history');
        historyContainer.innerHTML = '';

        const recentTransactions = this.transactions.slice(-20).reverse();
        
        recentTransactions.forEach(transaction => {
            const transactionItem = this.createTransactionItem(transaction);
            historyContainer.appendChild(transactionItem);
        });

        if (recentTransactions.length === 0) {
            historyContainer.innerHTML = '<p class="text-center text-muted">No transactions yet</p>';
        }
    }

    createTransactionItem(transaction) {
        const item = document.createElement('div');
        item.className = 'transaction-item fade-in';
        
        const iconClass = transaction.type === 'DEPOSIT' ? 'deposit' : 
                         transaction.type === 'WITHDRAW' ? 'withdraw' : 'transfer';
        
        const amountClass = transaction.type === 'WITHDRAW' ? 'negative' : '';
        
        item.innerHTML = `
            <div class="transaction-icon ${iconClass}">
                <i class="fas fa-${transaction.type === 'DEPOSIT' ? 'arrow-down' : 
                                   transaction.type === 'WITHDRAW' ? 'arrow-up' : 'exchange-alt'}"></i>
            </div>
            <div class="transaction-details">
                <div class="transaction-type">${transaction.type}</div>
                <div class="transaction-amount ${amountClass}">$${transaction.amount.toFixed(2)}</div>
            </div>
            <div class="transaction-meta">
                <div class="transaction-status ${transaction.status.toLowerCase()}">${transaction.status}</div>
                <div class="transaction-time">${transaction.timestamp.toLocaleTimeString()}</div>
            </div>
        `;
        
        return item;
    }

    updateThreadMonitor() {
        const totalThreads = Math.floor(Math.random() * 20) + 10;
        const runningThreads = Math.floor(Math.random() * totalThreads);
        const idleThreads = totalThreads - runningThreads;

        document.getElementById('total-threads').textContent = totalThreads;
        document.getElementById('running-threads').textContent = runningThreads;
        document.getElementById('idle-threads').textContent = idleThreads;

        this.updateThreadList(totalThreads, runningThreads);
    }

    updateThreadList(totalThreads, runningThreads) {
        const threadContainer = document.getElementById('thread-container');
        threadContainer.innerHTML = '';

        for (let i = 0; i < totalThreads; i++) {
            const threadId = `THREAD_${i.toString().padStart(3, '0')}`;
            const status = i < runningThreads ? 'running' : 'idle';
            const startTime = new Date(Date.now() - Math.random() * 300000);

            const threadItem = document.createElement('div');
            threadItem.className = 'thread-item fade-in';
            threadItem.innerHTML = `
                <div class="thread-info">
                    <span class="thread-id">${threadId}</span>
                    <span class="thread-status ${status}">${status.toUpperCase()}</span>
                </div>
                <div class="thread-meta">
                    <small>Started: ${startTime.toLocaleTimeString()}</small>
                </div>
            `;
            
            threadContainer.appendChild(threadItem);
        }
    }

    updatePerformanceMetrics() {
        const successfulTransactions = this.transactions.filter(t => t.status === 'SUCCESS').length;
        const totalTransactions = this.transactions.length;
        const successRate = totalTransactions > 0 ? (successfulTransactions / totalTransactions * 100).toFixed(1) : 0;

        document.getElementById('success-rate').textContent = `${successRate}%`;
        document.getElementById('avg-response-time').textContent = `${Math.floor(Math.random() * 100) + 50}ms`;
        document.getElementById('throughput').textContent = `${Math.floor(Math.random() * 50) + 10} txn/sec`;
    }

    generateSampleData() {
        const sampleNames = [
            'John Smith', 'Jane Doe', 'Bob Johnson', 'Alice Brown', 'Charlie Wilson'
        ];

        for (let i = 0; i < 5; i++) {
            const name = sampleNames[i] + ' ' + (i + 1);
            const balance = Math.floor(Math.random() * 10000) + 100;
            
            const accountNumber = 'ACC' + Date.now().toString().slice(-6) + i;
            const account = {
                accountNumber,
                holderName: name,
                balance: balance,
                status: 'ACTIVE',
                createdAt: new Date(),
                transactions: []
            };

            if (balance > 0) {
                const transaction = {
                    id: 'TXN_' + Date.now() + i,
                    type: 'DEPOSIT',
                    amount: balance,
                    description: 'Initial deposit',
                    status: 'SUCCESS',
                    timestamp: new Date(),
                    fromAccount: '',
                    toAccount: accountNumber
                };
                account.transactions.push(transaction);
                this.transactions.push(transaction);
            }

            this.accounts.push(account);
        }

        this.updateAccountsList();
        this.updateDashboard();
        this.showModal('Success', 'Generated 5 sample accounts');
    }

    clearAllData() {
        if (confirm('Are you sure you want to clear all data? This action cannot be undone.')) {
            this.accounts = [];
            this.transactions = [];
            
            this.updateAccountsList();
            this.updateTransactionHistory();
            this.updateDashboard();
            
            this.showModal('Success', 'All data has been cleared');
        }
    }

    showModal(title, content) {
        document.getElementById('modal-title').textContent = title;
        document.getElementById('modal-body').innerHTML = content;
        document.getElementById('modal-overlay').classList.add('active');
    }

    hideModal() {
        document.getElementById('modal-overlay').classList.remove('active');
    }

    startRealTimeUpdates() {
        setInterval(() => {
            this.updateDashboard();
            this.updateAccountsList();
            this.updateTransactionHistory();
        }, 10000);
    }

    loadInitialData() {
        const savedAccounts = localStorage.getItem('mtbs_accounts');
        const savedTransactions = localStorage.getItem('mtbs_transactions');
        
        if (savedAccounts) {
            this.accounts = JSON.parse(savedAccounts);
            this.accounts.forEach(account => {
                account.createdAt = new Date(account.createdAt);
                account.transactions.forEach(txn => {
                    txn.timestamp = new Date(txn.timestamp);
                });
            });
        }
        
        if (savedTransactions) {
            this.transactions = JSON.parse(savedTransactions);
            this.transactions.forEach(txn => {
                txn.timestamp = new Date(txn.timestamp);
            });
        }

        this.updateAccountsList();
        this.updateTransactionHistory();
        this.updateDashboard();
    }
}

// Initialize when DOM is loaded
document.addEventListener('DOMContentLoaded', () => {
    window.frontend = new MTBSFrontend();
    
    // Auto-save data every 30 seconds
    setInterval(() => {
        if (window.frontend) {
            localStorage.setItem('mtbs_accounts', JSON.stringify(window.frontend.accounts));
            localStorage.setItem('mtbs_transactions', JSON.stringify(window.frontend.transactions));
        }
    }, 30000);
});
