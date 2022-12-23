import os
import gc
import itertools
import json

from math import sqrt

import pandas as pd
import numpy as np

# from sklearn.model_selection import TimeSeriesSplit
from sklearn.metrics import mean_squared_error
from statsmodels.tsa.stattools import adfuller
from statsmodels.tsa.arima.model import ARIMA
import pmdarima as pm

data_types = {"arrivalRate": "avgArrivalRate", "avgServiceTime": "avgServerServiceTime"}

configurations = list(itertools.product([0, 1, 2, 3], repeat=3))


def load_data(folder: str, data_type: str):
    """
    Loads every csv file in the specified <folder>, that contains the <data_type> word in it, into a Pandas dataframe. Returns a list that contains dataframes that correspond to each file in the <folder>.

    Arguments:
        folder (str): folder that contains the files to load
        data_type (str): the type of file to open. The value of this argument needs to exist in the file name.
    """
    gc.collect()
    datasets = []
    for item in os.listdir(folder):
        if not data_type in item:
            continue

        print(f"Loading data from file {item}")
        data = pd.read_csv(f"{folder}/{item}")

        datasets.append(data)

    print("Finished loading data....")
    return datasets


# Create train and test sets
# Maybe replace with cross-validation (for time-series)
# For this, you can look at: https://stats.stackexchange.com/questions/14099/using-k-fold-cross-validation-for-time-series-model-selection
# And a solution is provided by this module/function: https://scikit-learn.org/stable/modules/generated/sklearn.model_selection.TimeSeriesSplit.html
def get_train_test(data: pd.DataFrame):
    # Use 70% of the data as training data
    train_df = data[: round(0.7 * len(data))]
    # Use 30% of the data as evaluation data
    test_df = data[round(0.7 * len(data)) :]

    return train_df, test_df


def train_model(train_df, config):
    """
    Creates an ARIMA model with p, q, d as specified by the provided <config> and trains the model on the provided
    data contained within <train_df>.

    Arguments:
        traing_df (Pandas.DataFrame): the data entries on which to train an ARIMA model
        config (tuple): values for p, q, and d to use to create an ARIMA model
    """
    model = ARIMA(train_df, order=config)
    fitted_model = model.fit()
    return fitted_model


def get_optimal_model(data, data_type):
    # auto_arima identitfies the most optimal parameters for the model and return a single fitted ARIMA model
    model = pm.auto_arima(
        data[
            data_type
        ],  # the time-series to which to fit the ARIMA model. Needs to be a one-dimensional array of floats, not containing np.nan or np.inf values
        start_p=1,
        start_q=1,
        test="adf",  # use adftest to find optimal 'd'
        max_p=5,
        max_q=5,  # maximum p and q
        m=1,  # frequency of series
        d=None,  # let model determine 'd'
        seasonal=False,  # no Seasonality
        trace=True,
        error_action="ignore",
        suppress_warnings=True,
        stepwise=True,
    )

    return model  # .to_dict()['order']


def get_best_config(results):
    """
    Returns the configuration (i.e. set of p, q, d values) that corresponds to the lowest sum of RMSE measurements
    between ARIMA model predictions and the actual values, stemming from the test_df, along with the lowest value
    itself.

    Arguments:
        results (dict): dictionary with keys being the different configuration sets (p, q, d) and values the measured
        RMSE values between ARIMA predictions and actual values
    """
    return min(
        [(config, sum(value)) for config, value in results.items()], key=lambda x: x[1]
    )


def find_best_config(datasets: list[str], data_type):
    """
    Finds the configuration that minimises the RMSE across all provided <datasets>.
    For each dataset from the provided <datasets>, it iterates over each configuration set, stored in the
    <configurations> list, creates and trains a model on the training set of the current dataset using the current
    configuration, uses the fitted model to make predictions and calculates the RMSE for the predictions and the
    actual values stored in the test set of the current dataset. Finally, it uses the measured RMSE values to derive
    the configuration that minimises RMSE across all datasets (i.e. minimises the average RMSE across datasets).

    Arguments:
        datasets (list[str]): list of datasets
        data_type (str): type of data that needs to be processed from each dataset
    """
    results = {}
    for index, df in enumerate(datasets):
        train_df, test_df = get_train_test(df)

        for config in configurations:
            str_config = str(config)
            if not str_config in results:
                results[str_config] = []
            fitted_model = train_model(train_df[data_type], config)
            predictions = fitted_model.forecast(steps=len(test_df))
            rmse = sqrt(mean_squared_error(test_df[data_type].values, predictions))
            results[str_config].append(rmse)

    best_config, best_rmse = get_best_config(results)
    return best_config, best_rmse


def offline_model_config():
    """
    Loads datasets stored in the training_data folder and determines which (p, q, d) ARIMA configuration minimises the
    RMSE for each dataset.
    """
    ds_arrival_rate = load_data("../../training_data", data_types["arrivalRate"])
    ds_service_time = load_data("../../training_data", data_types["avgServiceTime"])

    best_config_ar, best_rmse_ar = find_best_config(ds_arrival_rate, "arrivalRate")
    best_config_st, best_rmse_st = find_best_config(ds_service_time, "avgServiceTime")

    return {
        "arrival_rate": {"best_config": best_config_ar, "best_rmse": best_rmse_ar},
        "service_time": {"best_config": best_config_st, "best_rmse": best_rmse_st},
    }


def online_predictions(data_type, data=None, no_predictions=None):
    data_df = load_data("../../training_data", data_types[data_type])

    rmses = []
    for dataset in data_df:
        train_df, test_df = get_train_test(dataset)

        best_model = get_optimal_model(train_df, data_type)

        # Predictions is an array of predicted values and confindence intervals that correspond to each forecasted value
        predictions = best_model.predict(len(test_df), return_conf_int=True)
        rmse = sqrt(mean_squared_error(test_df[data_type].values, predictions[0].values))

        rmses.append(rmse)
    return rmses


if __name__ == "__main__":
    print('Results using offline model condifuration:')
    print(offline_model_config)

    print("\nResults using online model configuration for:")
    print("   - arrival rate")
    print(online_predictions("arrivalRate"))
    print("   - service time")
    print(online_predictions("avgServiceTime"))
