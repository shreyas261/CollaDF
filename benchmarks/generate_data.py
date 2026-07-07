import csv
import random

def generate_csv(filename="50M.csv", rows=50000000):
    departments = ["Engineering", "Sales", "Marketing", "HR", "Finance"]
    
    with open(filename, mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(["id", "department", "salary", "rating"])
        for i in range(rows):
            writer.writerow([
                i,
                random.choice(departments),
                random.randint(40000, 150000),
                round(random.uniform(1.0, 5.0), 1)
            ])
    print(f"Generated {rows} rows in {filename}")

if __name__ == "__main__":
    generate_csv()