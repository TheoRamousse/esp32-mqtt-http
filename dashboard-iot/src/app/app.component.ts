import { Component } from '@angular/core';
import { Data } from 'src/models/data';
import { ApiCallerService } from 'src/services/api-caller.service';

@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.scss'],
})
export class AppComponent {
  title = 'ng2-charts-demo';
  chart: any;

  chartOptions = {
    animationEnabled: true,
    theme: 'light2',
    title: {
      text: 'Températures',
    },
    axisX: {
      valueFormatString: 'DD/MM/YY - HH:MM',
    },
    axisY: {
      title: 'Température en °C',
    },
    toolTip: {
      shared: true,
    },
    legend: {
      cursor: 'pointer',
      itemclick: function (e: any) {
        if (
          typeof e.dataSeries.visible === 'undefined' ||
          e.dataSeries.visible
        ) {
          e.dataSeries.visible = false;
        } else {
          e.dataSeries.visible = true;
        }
        e.chart.render();
      },
    },
    data: [
      {
        type: 'line',
        showInLegend: true,
        name: '',
        dataPoints: [{ x: 1, y: 10 }],
      },
    ],
  };

  getChartInstance(chart: object) {
    this.chart = chart;
    setTimeout(this.updateChart, 1000);
  }

  updateChart = () => {
    this.chartOptions.data = [];

    var resultTempSoil: any[] = [];
    var resultTempWater: any[] = [];
    var resultConducSoil: any[] = [];

    this.apiCallerService.getData().forEach((el) => {
      resultTempSoil.push({ date: el.receivedAt, val: el.tempSoil });
      resultTempWater.push({ date: el.receivedAt, val: el.waterSoil });
      resultConducSoil.push({ date: el.receivedAt, val: el.conductSoil });
    });

    var dataPointsSoil: any[] = [];

    resultTempSoil.forEach((soil) => {
      dataPointsSoil.push({ x: new Date(soil.date), y: soil.val });
    });

    this.chartOptions.data.push({
      type: 'line',
      showInLegend: true,
      name: 'Température sol',
      dataPoints: dataPointsSoil,
    });

    var dataPointsWater: any[] = [];

    resultTempWater.forEach((water) => {
      dataPointsWater.push({ x: new Date(water.date), y: water.val });
    });

    this.chartOptions.data.push({
      type: 'line',
      showInLegend: true,
      name: 'Température eau',
      dataPoints: dataPointsWater,
    });

    var dataPointsConducSoil: any[] = [];

    resultConducSoil.forEach((conducSoil) => {
      dataPointsConducSoil.push({
        x: new Date(conducSoil.date),
        y: conducSoil.val,
      });
    });

    this.chartOptions.data.push({
      type: 'line',
      showInLegend: true,
      name: 'Conduc sol',
      dataPoints: dataPointsConducSoil,
    });

    this.chart.render();
    setTimeout(this.updateChart, 20000); //Chart updated every 1 second
  };

  constructor(private apiCallerService: ApiCallerService) {}
}
