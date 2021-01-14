import matplotlib.pyplot as plt
import numpy as np

def plot1(labels, values):
    
    

    x = np.arange(len(labels))  # the label locations
    width = 0.35  # the width of the bars

    fig, ax = plt.subplots()
    rects1 = ax.bar(x, values, width)

    # Add some text for labels, title and custom x-axis tick labels, etc.
    ax.set_ylabel('Czas (w sekundach)')
    ax.set_xlabel('Wielkość danych wejściowych')
    ax.set_xticks(x)
    ax.set_xticklabels(labels)


    def autolabel(rects):
        """Attach a text label above each bar in *rects*, displaying its height."""
        for rect in rects:
            height = rect.get_height()
            ax.annotate('{}'.format(height),
                        xy=(rect.get_x() + rect.get_width() / 2, height),
                        xytext=(0, 3),  # 3 points vertical offset
                        textcoords="offset points",
                        ha='center', va='bottom')


    autolabel(rects1)

    fig.tight_layout()
    fig.savefig('plot.png')

    plt.show()

def plot2(labels, values1, values2):

    x = np.arange(len(labels))  # the label locations
    width = 0.35  # the width of the bars

    fig, ax = plt.subplots()
    rects1 = ax.bar(x - width/2, values1, width, label='WAM')
    rects2 = ax.bar(x + width/2, values2, width, label='SWI-Prolog')

    # Add some text for labels, title and custom x-axis tick labels, etc.
    ax.set_ylabel('liczba inferencji')
    ax.set_xlabel('Wielkość danych wejściowych')
    ax.set_xticks(x)
    ax.set_xticklabels(labels)
    ax.legend()


    def autolabel(rects):
        """Attach a text label above each bar in *rects*, displaying its height."""
        for rect in rects:
            height = rect.get_height()
            ax.annotate('{}'.format(height),
                        xy=(rect.get_x() + rect.get_width() / 2, height),
                        xytext=(0, 3),  # 3 points vertical offset
                        textcoords="offset points",
                        ha='center', va='bottom')


    autolabel(rects1)
    autolabel(rects2)

    fig.tight_layout()
    fig.savefig('plot.png')

    plt.show()

# qsort50     746         3.03238 736         0
# qsort100    1895        7.61497 1830        0.002
# qsort200    4561        18.3112 4456        0.002
# qsort400    9931        40.1653 9742        0.007
# qsort800    22625       89.6112 22231       0.012
# nrev50      1330        4.86172 1331        0.001
# nrev100     5155        18.7514 5156        0.001
# nrev200     20305       79.0501 20306       0.003
# nrev400     80605       302.713 80606       0.016
# nrev800     321205      1388.81 321206      0.021
# con50       359         1.35255 311         0.003
# con100      709         4.30671 610         0.007
# con200      1409        14.5893 1210        0.019
# con400      2809        53.3144 2410        0.059
# con800      5609        205.324 4810        0.206
# sub10       5121        10.7481 4100        0.013
# sub11       10241       22.5665 8195        0.021
# sub12       20491       45.0457 16387       0.052
# sub13       40961       90.7641 32771       0.082
# sub14       81921       185.698 65539       0.164

def plotqsorti():
    labels = ['50', '100', '200', '400', '800']
    values1 = [746, 1895, 4561, 9931, 22625]
    values2 = [736, 1830, 4456, 9742, 22231]
    plot2(labels, values1, values2)

def plotqsorttw():
    labels = ['50', '100', '200', '400', '800']
    values = [3.03238, 7.61497, 18.3112, 40.1653, 89.6112]
    plot1(labels, values)

def plotqsortts():
    labels = ['50', '100', '200', '400', '800']
    values = [0, 0.002, 0.002, 0.007, 0.012]
    plot1(labels, values)

def plotnrevi():
    labels = ['50', '100', '200', '400', '800']
    values1 = [1330, 5155, 20305, 80605, 321205]
    values2 = [1331, 5156, 20306, 80606, 321206]
    plot2(labels, values1, values2)

def plotnrevtw():
    labels = ['50', '100', '200', '400', '800']
    values = [4.86172, 18.7514, 79.0501, 302.713, 1388.81]
    plot1(labels, values)

def plotnrevts():
    labels = ['50', '100', '200', '400', '800']
    values = [0.000, 0.001, 0.003, 0.016, 0.041]
    plot1(labels, values)

def plotconcati():
    labels = ['50', '100', '200', '400', '800']
    values1 = [359, 709, 1409, 2809, 5609]
    values2 = [311, 610, 1210, 2410, 4810]
    plot2(labels, values1, values2)

def plotconcattw():
    labels = ['50', '100', '200', '400', '800']
    values = [1.35255, 4.30671, 14.5893, 53.3144, 205.324]
    plot1(labels, values)

def plotconcatts():
    labels = ['50', '100', '200', '400', '800']
    values = [0.003, 0.007, 0.019, 0.059, 0.206]
    plot1(labels, values)

def plotsubsi():
    labels = ['10', '11', '12', '13', '14']
    values1 = [5121, 10241, 20491, 40961, 81921]
    values2 = [4100, 8195, 16387, 32771, 65539]
    plot2(labels, values1, values2)

def plotsubstw():
    labels = ['10', '11', '12', '13', '14']
    values = [10.7481, 22.5665, 45.0457, 90.7641, 185.698]
    plot1(labels, values)

def plotsubsts():
    labels = ['10', '11', '12', '13', '14']
    values = [0.013, 0.021, 0.052, 0.082, 0.164]
    plot1(labels, values)
