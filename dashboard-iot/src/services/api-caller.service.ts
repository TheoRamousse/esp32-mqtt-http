import { Injectable } from '@angular/core';
import { Data } from 'src/models/data';
import { HttpClient, HttpHeaders } from '@angular/common/http';
import { Observable } from 'rxjs';
import { environment } from 'src/environment';

@Injectable({
  providedIn: 'root',
})
export class ApiCallerService {
  constructor(private http: HttpClient) {}

  getData(): Observable<any> {
    return this.http.get(environment.apiBaseUrl + '/lorawan');
  }

  /*getData(): Data[] {
    return [
      new Data(2.3, 4.6, 7.8, Date.now()),
      new Data(4.3, 6.6, 8.8, Date.now()),
      new Data(6.3, 8.6, 9.8, Date.now()),
    ];
  }*/
}
