/*
    In this module we'll have all the logic related to the charts. We're
    going to export some functions that will be used in the data-manager
 */

/*
 First we import chartJS
     NOTE: This is a quick way to test the library. In the real project
     we need to see if we want to import the whole library
 */
import Chart from 'chart.js/auto';


// CONSTANTS
const maxPoints = 10;


// Select the canvas
const ctx = document.querySelector('#myChart').getContext('2d');

// Create the data object
const data = {
    labels: [],
    datasets: [{
       label: 'Serial',
       data: [],
       fill: true,
       borderColor: 'rgb(75, 192, 192)',
       tension: 0.1,
       spanGaps: true,
    }],
};

// Now we create the options object
const options = {
    responsive: true,
    plugins: {
        legend: {
            position: 'top',
        },

        title: {
            display: true,
            text: 'Serial data',
        },
    },

    scales: {
            xAxes: [ {ticks: {autoSkip:true, maxTicksLimit:10} } ]
    }

};

// Here we create the chart
const chart = new Chart(ctx, {
    type: 'line',
    data: data,
    options: options,
});


// Now we're going to create some fn to update the chart data

function updateChart(chart, data, label){
    chart.data.labels.push(label);

    chart.data.datasets.forEach(dataset => {
        dataset.data.push(data);
    });

    if(chart.data.labels.length > maxPoints){
        chart.data.labels.shift();
        chart.data.datasets[0].data.shift();
    }

    chart.update();
}

export {updateChart, chart};