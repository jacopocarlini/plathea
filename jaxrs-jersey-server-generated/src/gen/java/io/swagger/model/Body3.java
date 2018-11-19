/*
 * Plathea
 * Rest APIs Plathea
 *
 * OpenAPI spec version: 1.0.1-oas3
 * 
 *
 * NOTE: This class is auto generated by the swagger code generator program.
 * https://github.com/swagger-api/swagger-codegen.git
 * Do not edit the class manually.
 */

package io.swagger.model;

import java.util.Objects;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.annotation.JsonCreator;
import io.swagger.v3.oas.annotations.media.Schema;
import java.io.File;
import java.util.ArrayList;
import java.util.List;
import javax.validation.constraints.*;

/**
 * Body3
 */
@javax.annotation.Generated(value = "io.swagger.codegen.v3.generators.java.JavaJerseyServerCodegen", date = "2018-11-18T18:02:56.242Z[GMT]")public class Body3   {
  @JsonProperty("filename")
  private List<File> filename = null;

  public Body3 filename(List<File> filename) {
    this.filename = filename;
    return this;
  }

  public Body3 addFilenameItem(File filenameItem) {
    if (this.filename == null) {
      this.filename = new ArrayList<File>();
    }
    this.filename.add(filenameItem);
    return this;
  }

  /**
   * Get filename
   * @return filename
   **/
  @JsonProperty("filename")
  @Schema(description = "")
  public List<File> getFilename() {
    return filename;
  }

  public void setFilename(List<File> filename) {
    this.filename = filename;
  }


  @Override
  public boolean equals(java.lang.Object o) {
    if (this == o) {
      return true;
    }
    if (o == null || getClass() != o.getClass()) {
      return false;
    }
    Body3 body3 = (Body3) o;
    return Objects.equals(this.filename, body3.filename);
  }

  @Override
  public int hashCode() {
    return Objects.hash(filename);
  }


  @Override
  public String toString() {
    StringBuilder sb = new StringBuilder();
    sb.append("class Body3 {\n");
    
    sb.append("    filename: ").append(toIndentedString(filename)).append("\n");
    sb.append("}");
    return sb.toString();
  }

  /**
   * Convert the given object to string with each line indented by 4 spaces
   * (except the first line).
   */
  private String toIndentedString(java.lang.Object o) {
    if (o == null) {
      return "null";
    }
    return o.toString().replace("\n", "\n    ");
  }
}
