import itertools


# To generate a sample of combinations to test the program
def generate_combinations_to_file(n, file_name, sample_size=10):
    numbers = list(range(1, n + 1))
    choices = [(str(x), f"~{x}") for x in numbers]
    combinations = itertools.product(*choices)

    with open(file_name, "w") as file:
        for idx, combo in enumerate(combinations):
            file.write(" ".join(combo) + "\n")
            if idx >= sample_size - 1:  # Sample size limit
                break


generate_combinations_to_file(20, "combinations.txt", sample_size=1024 * 1024)
