import { Injectable } from '@angular/core';
import { Data } from 'src/models/data';
import { HttpClient, HttpHeaders } from '@angular/common/http';

@Injectable({
  providedIn: 'root',
})
export class ApiCallerService {
  constructor(private http: HttpClient) {}

  /*getData() {
    const headers = new HttpHeaders({
      'Content-Type': 'application/json; charset=utf-8',
      Authorization:
        'Bearer NNSXS.AFXIMSE6QXHFGBFXSYHMQQ6XFXJKDAOKRNFGHHI.N4WWBDZ7B7TNJA4IKJ6DGZAS6PNSRQBXZSWPFZT5ZSON52NGJW2A',
    });

    this.http
      .get(
        'https://eu1.cloud.thethings.network/api/v3/as/applications/soulmbengue-app-lorawansrv-1/packages/storage/uplink_message',
        { headers }
      )
      .subscribe(
        (res) => {
          console.log(res);
        },
        (err) => {
          console.log(err.error.text);
        }
      );
  }*/

  getData(): Data[] {
    return [
      new Data(2.3, 4.6, 7.8, Date.now()),
      new Data(4.3, 6.6, 8.8, Date.now()),
      new Data(6.3, 8.6, 9.8, Date.now()),
    ];
  }
}
