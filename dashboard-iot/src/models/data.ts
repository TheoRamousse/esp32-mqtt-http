export class Data {
  public conductSoil;
  public tempSoil;
  public waterSoil;
  public receivedAt;

  constructor(
    conductSoil: number,
    tempSoil: number,
    waterSoil: number,
    receivedAt: number
  ) {
    this.conductSoil = conductSoil;
    this.tempSoil = tempSoil;
    this.waterSoil = waterSoil;
    this.receivedAt = receivedAt;
  }
}
