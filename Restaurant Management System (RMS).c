#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_ORDERS 100
#define MAX_TABLES 20
#define MAX_ITEMS 12
#define MAX_NAME 50
#define MAX_MOBILE 15
#define LINE_LENGTH 1000

// Menu items structure
typedef struct {
    char name[30];
    int price;
} MenuItem;

// Order item structure
typedef struct {
    int item_id;
    int quantity;
} OrderItem;

// Single order structure
typedef struct {
    char customer_name[MAX_NAME];
    char mobile[MAX_MOBILE];
    int table_no;           // -1 for parcel
    int parcel_no;          // -1 if not parcel
    OrderItem items[MAX_ITEMS];
    int item_count;
    int total_amount;
    int status;             // 0=Pending,1=Preparing,2=Ready,3=Served
    int order_id;
} Order;

// Stack for ready orders (LIFO - last ready first served)
typedef struct {
    Order orders[MAX_ORDERS];
    int top;
} ReadyStack;

// Queue for pending/preparing orders (FIFO)
typedef struct {
    Order orders[MAX_ORDERS];
    int front, rear;
} OrderQueue;

// Table booking structure
typedef struct {
    char customer_name[MAX_NAME];
    int status;  // 0=free, 1=booked
} Table;

// Global variables
MenuItem menu[MAX_ITEMS];
Table tables[MAX_TABLES];
OrderQueue pendingQueue;
ReadyStack readyStack;
int next_order_id = 1;
int next_parcel_no = 1;

// Function prototypes
void init_menu();
void init_tables();
void init_queues();
int is_queue_empty(OrderQueue* q);
int is_queue_full(OrderQueue* q);
void enqueue(OrderQueue* q, Order order);
Order dequeue(OrderQueue* q);
void push_ready(ReadyStack* s, Order order);
Order pop_ready(ReadyStack* s);
int is_stack_empty(ReadyStack* s);
void display_menu();
int get_item_price(int item_id);
void take_new_order();
void update_order_status();
void process_payment();
void show_all_orders();
void show_ready_orders();
void serve_order();
void book_table();
void show_table_status();
void generate_receipt(Order order);
void save_orders_to_file();
void load_orders_from_file();
void save_tables_to_file();
void load_tables_from_file();

// Initialize menu items
void init_menu() {
    strcpy(menu[0].name, "Rice"); menu[0].price = 10;
    strcpy(menu[1].name, "Murgi Jhal fry"); menu[1].price = 50;
    strcpy(menu[2].name, "Murgi vuna half"); menu[2].price = 40;
    strcpy(menu[3].name, "Murgi vuna full"); menu[3].price = 80;
    strcpy(menu[4].name, "Alu Vorta"); menu[4].price = 10;
    strcpy(menu[5].name, "Pangash Mach"); menu[5].price = 50;
    strcpy(menu[6].name, "Rui Mach"); menu[6].price = 60;
    strcpy(menu[7].name, "Dim Vuna"); menu[7].price = 25;
    strcpy(menu[8].name, "Vaji"); menu[8].price = 20;
    strcpy(menu[9].name, "Shobji"); menu[9].price = 20;
    strcpy(menu[10].name, "Patla Dal"); menu[10].price = 0;
    strcpy(menu[11].name, "Others"); menu[11].price = 0;
}

// Initialize tables
void init_tables() {
    int i;
    for(i = 0; i < MAX_TABLES; i++) {
        tables[i].status = 0;
        strcpy(tables[i].customer_name, "Free");
    }
}

// Initialize queues and stack
void init_queues() {
    pendingQueue.front = pendingQueue.rear = 0;
    readyStack.top = -1;
}

// Queue empty check
int is_queue_empty(OrderQueue* q) {
    return q->front == q->rear;
}

// Queue full check
int is_queue_full(OrderQueue* q) {
    return (q->rear + 1) % MAX_ORDERS == q->front;
}

// Enqueue operation
void enqueue(OrderQueue* q, Order order) {
    if(is_queue_full(q)) {
        printf("Queue is full! Cannot add order.\n");
        return;
    }
    q->orders[q->rear] = order;
    q->rear = (q->rear + 1) % MAX_ORDERS;
}

// Dequeue operation
Order dequeue(OrderQueue* q) {
    Order empty = {0};
    if(is_queue_empty(q)) {
        printf("Queue is empty!\n");
        return empty;
    }
    Order order = q->orders[q->front];
    q->front = (q->front + 1) % MAX_ORDERS;
    return order;
}

// Stack push for ready orders
void push_ready(ReadyStack* s, Order order) {
    if(s->top >= MAX_ORDERS - 1) {
        printf("Ready stack is full!\n");
        return;
    }
    s->orders[++s->top] = order;
}

// Stack pop for ready orders
Order pop_ready(ReadyStack* s) {
    Order empty = {0};
    if(is_stack_empty(s)) {
        printf("No ready orders!\n");
        return empty;
    }
    return s->orders[s->top--];
}

// Stack empty check
int is_stack_empty(ReadyStack* s) {
    return s->top == -1;
}

// Display menu
void display_menu() {
    int i;
    printf("\n=== RESTAURANT MENU ===\n");
    for(i = 0; i < 11; i++) {
        printf("%2d) %s - %d Taka\n", i+1, menu[i].name, menu[i].price);
    }
    printf("=======================\n");
}

// Get item price
int get_item_price(int item_id) {
    if(item_id >= 1 && item_id <= 11) {
        return menu[item_id-1].price;
    }
    return 0;
}

// Take new order
void take_new_order() {
    Order new_order = {0};
    new_order.order_id = next_order_id++;
    new_order.status = 0;  // Pending

    printf("\n--- NEW ORDER ---\n");
    printf("Customer Name: ");
    scanf("%s", new_order.customer_name);

    printf("Mobile: ");
    scanf("%s", new_order.mobile);

    printf("Table or Parcel? (1=Table, 2=Parcel): ");
    int choice;
    scanf("%d", &choice);

    if(choice == 1) {
        // Table booking
        printf("Table No (1-%d): ", MAX_TABLES);
        scanf("%d", &new_order.table_no);
        new_order.parcel_no = -1;
        if(new_order.table_no > 0 && new_order.table_no <= MAX_TABLES) {
            strcpy(tables[new_order.table_no-1].customer_name, new_order.customer_name);
            tables[new_order.table_no-1].status = 1;
        }
    } else {
        // Parcel
        new_order.table_no = -1;
        new_order.parcel_no = next_parcel_no++;
    }

    // Add items
    new_order.item_count = 0;
    new_order.total_amount = 0;
    display_menu();

    while(1) {
        printf("\nItem ID (1-11, 0 to finish): ");
        int item_id, qty;
        scanf("%d", &item_id);
        if(item_id == 0) break;

        printf("Quantity: ");
        scanf("%d", &qty);

        if(new_order.item_count < MAX_ITEMS) {
            new_order.items[new_order.item_count].item_id = item_id;
            new_order.items[new_order.item_count].quantity = qty;
            new_order.total_amount += get_item_price(item_id) * qty;
            new_order.item_count++;
        }
    }

    enqueue(&pendingQueue, new_order);
    printf("Order #%d added to pending queue! Total: %d Taka\n",
           new_order.order_id, new_order.total_amount);
}

// Update order status
void update_order_status() {
    if(is_queue_empty(&pendingQueue)) {
        printf("No pending orders!\n");
        return;
    }

    printf("\n--- UPDATE STATUS ---\n");
    printf("Enter Order ID to update: ");
    int order_id;
    scanf("%d", &order_id);

    int found = 0;
    int i;
    for(i = pendingQueue.front; i != pendingQueue.rear;
        i = (i + 1) % MAX_ORDERS) {
        if(pendingQueue.orders[i].order_id == order_id) {
            found = 1;
            printf("Current Status: ");
            switch(pendingQueue.orders[i].status) {
                case 0: printf("Pending\n"); break;
                case 1: printf("Preparing\n"); break;
                case 2: printf("Ready\n"); break;
            }

            printf("New Status (0=Pending,1=Preparing,2=Ready): ");
            int new_status;
            scanf("%d", &new_status);

            if(new_status == 2) {  // Move to ready stack
                Order ready_order = pendingQueue.orders[i];
                ready_order.status = 2;
                push_ready(&readyStack, ready_order);
                printf("Order moved to ready stack!\n");
            } else {
                pendingQueue.orders[i].status = new_status;
                printf("Status updated!\n");
            }
            break;
        }
    }

    if(!found) {
        printf("Order not found!\n");
    }
}

// Process payment
void process_payment() {
    printf("\n--- PAYMENT ---\n");
    printf("Enter Order ID: ");
    int order_id;
    scanf("%d", &order_id);

    // Check in pending queue
    int found = 0;
    int i;
    for(i = pendingQueue.front; i != pendingQueue.rear;
        i = (i + 1) % MAX_ORDERS) {
        if(pendingQueue.orders[i].order_id == order_id) {
            found = 1;
            printf("Payment Method (1=Bkash/Nagad, 2=Cash): ");
            int method;
            scanf("%d", &method);
            if(method == 1) {
                char number[MAX_MOBILE];
                printf("Enter Bkash/Nagad number: ");
                scanf("%s", number);
                printf("Payment confirmed via %s\n", number);
            } else {
                printf("Cash payment confirmed!\n");
            }
            break;
        }
    }

    if(!found) {
        printf("Order not found!\n");
    }
}

// Show all orders
void show_all_orders() {
    printf("\n--- ALL ORDERS ---\n");
    int count = 0;
    int i;
    for(i = pendingQueue.front; i != pendingQueue.rear;
        i = (i + 1) % MAX_ORDERS) {
        Order ord = pendingQueue.orders[i];
        printf("ID:%d | %s | Table:%d | Status:%d | Total:%d\n",
               ord.order_id, ord.customer_name, ord.table_no,
               ord.status, ord.total_amount);
        count++;
    }
    printf("Total pending orders: %d\n", count);
}

// Show ready orders (stack)
void show_ready_orders() {
    printf("\n--- READY ORDERS (LIFO) ---\n");
    if(is_stack_empty(&readyStack)) {
        printf("No ready orders!\n");
        return;
    }

    int i;
    for(i = readyStack.top; i >= 0; i--) {
        Order ord = readyStack.orders[i];
        printf("ID:%d | %s | Table:%d | Total:%d\n",
               ord.order_id, ord.customer_name, ord.table_no, ord.total_amount);
    }
}

// Serve order (pop from stack)
void serve_order() {
    Order served = pop_ready(&readyStack);
    if(served.order_id == 0) return;

    served.status = 3;  // Served

    printf("\n--- ORDER SERVED ---\n");
    generate_receipt(served);

    // Free table if booked
    if(served.table_no > 0 && served.table_no <= MAX_TABLES) {
        tables[served.table_no-1].status = 0;
        strcpy(tables[served.table_no-1].customer_name, "Free");
    }
}

// Book table
void book_table() {
    printf("\n--- TABLE BOOKING ---\n");
    printf("Customer Name: ");
    char name[MAX_NAME];
    scanf("%s", name);

    printf("Table No (1-%d): ", MAX_TABLES);
    int table_no;
    scanf("%d", &table_no);

    if(table_no > 0 && table_no <= MAX_TABLES && tables[table_no-1].status == 0) {
        strcpy(tables[table_no-1].customer_name, name);
        tables[table_no-1].status = 1;
        printf("Table %d booked for %s\n", table_no, name);
    } else {
        printf("Table not available!\n");
    }
}

// Show table status
void show_table_status() {
    printf("\n--- TABLE STATUS ---\n");
    int i;
    for(i = 0; i < MAX_TABLES; i++) {
        printf("Table %2d: %s\n", i+1, tables[i].customer_name);
    }
}

// Generate receipt
void generate_receipt(Order order) {
    printf("\n==================== RECEIPT ====================\n");
    printf("Customer: %s\n", order.customer_name);
    printf("Mobile: %s\n", order.mobile);
    if(order.table_no > 0) {
        printf("Table No: %d\n", order.table_no);
    } else {
        printf("Parcel No: %d\n", order.parcel_no);
    }
    printf("Order ID: %d\n", order.order_id);
    printf("Status: ");
    switch(order.status) {
        case 0: printf("Pending\n"); break;
        case 1: printf("Preparing\n"); break;
        case 2: printf("Ready\n"); break;
        case 3: printf("Served\n"); break;
    }

    printf("\nITEMS:\n");
    int i;
    for(i = 0; i < order.item_count; i++) {
        int item_id = order.items[i].item_id;
        printf("%s x%d = %d Taka\n",
               menu[item_id-1].name,
               order.items[i].quantity,
               get_item_price(item_id) * order.items[i].quantity);
    }
    printf("===============================================\n");
    printf("TOTAL AMOUNT: %d Taka\n", order.total_amount);
    printf("===============================================\n");
}

// Save orders to file
void save_orders_to_file() {
    FILE *fp = fopen("orders.dat", "wb");
    if(fp == NULL) return;

    fwrite(&next_order_id, sizeof(int), 1, fp);
    fwrite(&next_parcel_no, sizeof(int), 1, fp);
    fwrite(&pendingQueue, sizeof(OrderQueue), 1, fp);
    fwrite(&readyStack, sizeof(ReadyStack), 1, fp);

    fclose(fp);
    printf("Orders saved to file.\n");
}

// Load orders from file
void load_orders_from_file() {
    FILE *fp = fopen("orders.dat", "rb");
    if(fp == NULL) return;

    fread(&next_order_id, sizeof(int), 1, fp);
    fread(&next_parcel_no, sizeof(int), 1, fp);
    fread(&pendingQueue, sizeof(OrderQueue), 1, fp);
    fread(&readyStack, sizeof(ReadyStack), 1, fp);

    fclose(fp);
    printf("Orders loaded from file.\n");
}

// Save tables to file
void save_tables_to_file() {
    FILE *fp = fopen("tables.dat", "wb");
    if(fp == NULL) return;
    fwrite(tables, sizeof(Table), MAX_TABLES, fp);
    fclose(fp);
}

// Load tables from file
void load_tables_from_file() {
    FILE *fp = fopen("tables.dat", "rb");
    if(fp == NULL) return;
    fread(tables, sizeof(Table), MAX_TABLES, fp);
    fclose(fp);
}

// Main menu function
void show_main_menu() {
    printf("\n========== RESTAURANT MANAGEMENT SYSTEM ==========\n");
    printf("1.  Display Menu\n");
    printf("2.  Take New Order\n");
    printf("3.  Update Order Status\n");
    printf("4.  Process Payment\n");
    printf("5.  Show All Orders\n");
    printf("6.  Show Ready Orders\n");
    printf("7.  Serve Order\n");
    printf("8.  Book Table\n");
    printf("9.  Show Table Status\n");
    printf("10. Save Data\n");
    printf("11. Load Data\n");
    printf("0.  Exit\n");
    printf("==================================================\n");
    printf("Enter choice: ");
}

int main() {
    init_menu();
    init_tables();
    init_queues();
    load_orders_from_file();
    load_tables_from_file();

    int choice;
    while(1) {
        show_main_menu();
        scanf("%d", &choice);

        switch(choice) {
            case 1: display_menu(); break;
            case 2: take_new_order(); break;
            case 3: update_order_status(); break;
            case 4: process_payment(); break;
            case 5: show_all_orders(); break;
            case 6: show_ready_orders(); break;
            case 7: serve_order(); break;
            case 8: book_table(); break;
            case 9: show_table_status(); break;
            case 10:
                save_orders_to_file();
                save_tables_to_file();
                break;
            case 11:
                load_orders_from_file();
                load_tables_from_file();
                break;
            case 0:
                save_orders_to_file();
                save_tables_to_file();
                printf("Thank you for using RMS!\n");
                return 0;
            default: printf("Invalid choice!\n");
        }
    }

    return 0;
}
