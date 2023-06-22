import { NgModule } from '@angular/core';
import { BrowserModule } from '@angular/platform-browser';
import { HttpClientModule } from '@angular/common/http';
import { AppRoutingModule } from './app-routing.module';
import { AppComponent } from './app.component';
import { CanvasJSAngularChartsModule } from '@canvasjs/angular-charts';

@NgModule({
  declarations: [AppComponent],
  imports: [
    BrowserModule,
    AppRoutingModule,
    HttpClientModule,
    CanvasJSAngularChartsModule,
  ],
  providers: [],
  bootstrap: [AppComponent],
})
export class AppModule {}
