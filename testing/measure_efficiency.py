import matplotlib.pyplot as plt
import pymongo
from sys import argv
from time import sleep

def parse_argv(argv):
    try:
        _, uri, db_name, collection_name, sp, iter, plot_name = argv
        return uri, db_name, collection_name, int(sp)/1000, int(iter), plot_name
    except ValueError:
        print("Invalid arguments!")
        print("usage: ./measure_efficiency <database-uri> <database-name> <collection-name> <sampling-period-(ms)> <total-iterations <save-file-name>")
        print("N.B. uri has to be in mongodb format (e.g. mongodb://localhost:27017/)")
        exit()


if __name__ == "__main__":
    uri, db_name, collection_name, sp, iter, plot_name = parse_argv(argv)
    db = pymongo.MongoClient(uri)[db_name]
    collection = db[collection_name]
    print(f"Ready to measure document count at {uri}{db_name}[{collection_name}] every {sp} seconds")

    document_counts = []
    document_counts.append(collection.count_documents({}))
    start_count = document_counts[-1]
    pages_delta = []

    # while (collection.count_documents({}) == start_count):
        # sleep(sp)

    prev_c, new_c = start_count, start_count
    for i in range(iter - 1):
        new_c = collection.count_documents({})
        print(new_c)
        document_counts.append(new_c)
        pages_delta.append(new_c - prev_c)
        prev_c = new_c
        sleep(sp)

    with open(plot_name, "w") as output:
        output.write(str(document_counts)[1:-1])

    fig = plt.subplots(figsize=(12, 8))
    plt.plot(document_counts)
    plt.xlabel(f"Step ({sp})")
    plt.ylabel("Page collection size (pages)")
    plt.ylim(bottom=0)
    plt.xlim(left=1)
    plt.title("Total page collection size")
    plt.savefig(f"{plot_name}_counts")

    fig = plt.subplots(figsize=(12, 8))
    plt.plot(pages_delta)
    plt.xlabel(f"Step ({sp})")
    plt.ylabel("Pages parsed per single step (pages)")
    plt.ylim(bottom=0)
    plt.xlim(left=1)
    plt.title("Pages parsed per single step")
    plt.savefig(f"{plot_name}_deltas")
