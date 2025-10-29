import math

def compute_pi_zeta(iterations):
    if iterations < 1:
        return 0.0
    
    total_sum = 0.0
    for k in range(1, iterations + 1):
        total_sum += 1.0 / (k * k)
    
    return math.sqrt(6 * total_sum)

def calculate_error(computed_pi, reference_pi=math.pi):
    absolute_error = abs(computed_pi - reference_pi)
    relative_error = absolute_error / reference_pi
    return relative_error * 100.0

def main():
    print("Auto test")
    test_iterations = [10, 100, 1000, 10000]
    
    for n in test_iterations:
        computed_pi = compute_pi_zeta(n)
        error_percent = calculate_error(computed_pi)
        
        print(f"Iteration: {n} | Pi: {computed_pi:.10f} | Error percent is: {error_percent:.4f}%", end=" ")
        
        if error_percent < 0.1:
            print("Hurray, it's done!")
        else:
            print("Mistakes...")
    
    print("\nInteractive mode")
    
    try:
        n = int(input("Enter the number for iterations for calculating Pi: "))
        
        if n <= 0:
            print("Warning! It's expected to get an integer number for iterations!")
            return
        
        computed_pi = compute_pi_zeta(n)
        error_percent = calculate_error(computed_pi)
        
        print(f"Computed value of Pi: {computed_pi}")
        print(f"Error percent: {error_percent:.4f}%")
        
        if error_percent >= 0.1:
            print("Warning!Accuracy is under 0.1%. Increase the number of iterations!")
            
    except ValueError:
        print("Error!! Enter an integer number!")

if __name__ == "__main__":
    main()